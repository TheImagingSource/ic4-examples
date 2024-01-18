
#include <ic4/ic4.h>

#include <console-helper.h>

#include <mutex>
#include <condition_variable>
#include <chrono>

struct RealWorld
{
	std::chrono::milliseconds sceneSetupDuration_;

	std::mutex mtx_;
	int64_t current_frame_id_;
	std::condition_variable cv_;
	std::chrono::high_resolution_clock::time_point wait_end_;

	RealWorld(std::chrono::milliseconds sceneSetupDuration)
		: sceneSetupDuration_(sceneSetupDuration)
	{
		reset();
	}

	void beginSetupScene(int64_t frame_id)
	{
		std::unique_lock<std::mutex> lck(mtx_);

		if (frame_id > current_frame_id_)
		{
			current_frame_id_ = frame_id;
			wait_end_ = std::chrono::high_resolution_clock::now() + sceneSetupDuration_;
			cv_.notify_one();
		}
	}

	void waitSetupSceneCompletion()
	{
		std::chrono::high_resolution_clock::time_point wait_end;

		{
			std::unique_lock<std::mutex> lck(mtx_);

			while (wait_end_ == std::chrono::high_resolution_clock::time_point())
			{
				cv_.wait(lck,
					[this]()
					{
						return wait_end_ != std::chrono::high_resolution_clock::time_point();
					}
				);
			}

			wait_end = wait_end_;
			wait_end_ = {};
		}

		std::this_thread::sleep_until(wait_end);
	}

	void reset()
	{
		std::unique_lock<std::mutex> lck(mtx_);
		current_frame_id_ = -1;
		wait_end_ = {};
	}
};

static void runTest(ic4::Grabber& grabber, RealWorld& realWorld, int numCycles, ic4::Property* eventExposureEnd)
{
	auto map = grabber.devicePropertyMap();

	if (eventExposureEnd != nullptr)
	{
		// Register a notification handler for EventExposureEnd
		eventExposureEnd->eventAddNotification(
			[&map, &realWorld](auto&)
			{
				// Extract frame ID from event data
				auto fid = map.getValueInt64(ic4::PropId::EventExposureEndFrameID);

				// Request real world scene-setup for next frame
				// At this time, exposure is complete, but the image is still being transmitted.
				// If we waited for this call until the image is transmitted completely, we would waste time.
				realWorld.beginSetupScene(fid + 1);
			}
		);

		// Enable EventExposureEnd event notification
		map.setValue(ic4::PropId::EventSelector, "ExposureEnd");
		map.setValue(ic4::PropId::EventNotification, "On");
	}
	else
	{
		// Disable EventExposureEnd event notification
		map.setValue(ic4::PropId::EventSelector, "ExposureEnd");
		map.setValue(ic4::PropId::EventNotification, "Off");
	}

	// An event to wait for an image being received by the sink.
	// This event indicates that a new image can be triggered, therefore it is initially set.
	std::mutex imageReceivedMutex;
	std::condition_variable imageReceivedCv;
	bool imageReceived = true;

	struct Listener : ic4::QueueSinkListener
	{
		std::mutex& mtx_;
		std::condition_variable& cv_;
		bool& image_received_;
		RealWorld& realWorld_;
		ic4::PropertyMap map_;

		Listener(std::mutex& mtx, std::condition_variable& cv, bool& ir, RealWorld& rw, ic4::PropertyMap map)
			: mtx_(mtx)
			, cv_(cv)
			, image_received_(ir)
			, realWorld_(rw)
			, map_(map)
		{
		}

		void framesQueued(ic4::QueueSink& sink) final
		{
			auto buffer = sink.popOutputBuffer();

			{
				// Notify the test thread that an image was received, and it can proceed.
				std::unique_lock<std::mutex> lck(mtx_);
				image_received_ = true;
				cv_.notify_one();
			}

			std::cout << ".";

			auto fid = buffer->metaData().device_frame_number;

			// Request real world scene-setup for next frame.
			// This call will be ignored if the setup was already requested by the EventExposureEnd notification handler.
			realWorld_.beginSetupScene(fid + 1);
		}
	};

	auto listener = std::make_shared<Listener>(imageReceivedMutex, imageReceivedCv, imageReceived, realWorld, map);
	auto sink = ic4::QueueSink::create(listener);

	// Setup stream
	grabber.streamSetup(sink);

	// Request real-world scene for first frame.
	realWorld.beginSetupScene(0);

	std::cout << "Running " << numCycles << " cycles..." << std::endl;
	auto begin = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < numCycles; ++i)
	{
		// Wait for the previous image to be received (minimum cycle time).
		{
			std::unique_lock<std::mutex> lck(imageReceivedMutex);

			while (!imageReceived)
			{
				imageReceivedCv.wait(lck, [&imageReceived]() -> bool { return imageReceived; });
			}

			imageReceived = false;
		}

		// Wait for the scene setup to be completed.
		realWorld.waitSetupSceneCompletion();

		// Issue software trigger after both the real-world scene setup is completed and the previous image was received.
		map.executeCommand(ic4::PropId::TriggerSoftware);
	}

	auto end = std::chrono::high_resolution_clock::now();

	grabber.streamStop();

	auto dt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	std::cout << std::endl;
	std::cout << "Processed " << numCycles << " cycles in " << dt_ms << " ms. (" << (numCycles * 1000 / dt_ms) << " cycles/sec)" << std::endl;
}

int main()
{
	// Initialize the library with sensible defaults:
	// - Throw exceptions on errors
	// - Log errors and warnings from API calls
	// - Log to stdout and Windows debug log
	ic4::InitLibraryConfig libraryConfig =
	{
		ic4::ErrorHandlerBehavior::Throw,
		ic4::LogLevel::Warning,
		ic4::LogLevel::Off,
		ic4::LogTarget::StdOut | ic4::LogTarget::WinDebug
	};
	ic4::initLibrary(libraryConfig);
	// Automatically call exitLibrary when returning from main
	std::atexit(ic4::exitLibrary);

	try
	{
		// Let the user select a camera
		auto device_list = ic4::DeviceEnum::enumDevices();
		auto it = ic4_examples::console::select_from_list(device_list);
		if (it == device_list.end())
		{
			return -1;
		}

		// Create grabber and open device
		ic4::Grabber grabber;
		grabber.deviceOpen(*it);

		/**
		 * GigEVision and USB3Vision devices can send asynchronous events to applications
		 * This example shows how to receive events EventExposureEnd, which indicates that the camera has completed
		 * the integration time and will start transmitting the image data.
		 *
		 * This event is useful when synchronizing real-world activities with camera operation, for example moving
		 * something while the image is being transmitted.
		 *
		 * Events are configured and received through the device property map.
		 * The following shows an excerpt from the device property map of a device supporting EventExposureEnd:
		 *
		 * - EventControl
		 *   - EventSelector
		 *   - EventNotification[EventSelector]
		 *   - EventExposureEndData
		 *     - EventExposureEnd
		 *     - EventExposureEndTimestamp
		 *     - EventExposureEndFrameID
		 *
		 * To receive notifications for a specific event, two steps have to be taken:
		 *
		 * First, the device has to be configured to send generate the specific event. To enable the EventExposureEnd event, set the
		 * "EventSelector" enumeration property to "EventExposureEnd", and then set the "EventNotification" enumeration property to "On".
		 *
		 * Second, a property notification handler has to be registered for the property representing the event.
		 * The EventExposureEnd is represented by the integer property "EventExposureEnd". This property has no function other
		 * than being invalidated and thus having its notification raised when the device sends the event.
		 *
		 * Event parameters are grouped with the event property in a property category with "Data" appended to the event's name,
		 * in our case "EventExposureEndData". The category contains the integer properties "EventExposureEndTimestamp"
		 * and "EventExposureEndFrameID", which provide the time stamp and frame ID of the event.
		 * Event argument properties should only be read inside the event notification function to avoid data races.
		 */

		 // Get EventExposureEnd event property
		auto eventExposureEnd = grabber.devicePropertyMap().find(ic4::PropId::EventExposureEnd);

		auto map = grabber.devicePropertyMap();

		// Reset all camera settings to default so that prior configuration does not interfere with this program
		map.setValue(ic4::PropId::UserSetDefault, "Default");
		map.executeCommand(ic4::PropId::UserSetLoad);

		// Configure a constant exposure time
		map.setValue(ic4::PropId::ExposureAuto, "Off");
		map.setValue(ic4::PropId::ExposureTime, 1000.0);

		// Enable trigger mode
		map.setValue(ic4::PropId::TriggerMode, "On");

		// Create our "real world" with a next-frame setup time of 40 ms
		RealWorld realWorld(std::chrono::milliseconds(40));

		// Run test without supplying EventExposureEnd event
		std::cout << "Test WITHOUT EventExposureEnd" << std::endl;
		runTest(grabber, realWorld, 50, nullptr);

		// Reset real world for next test run
		realWorld.reset();

		// Run test with registered notification handler for EventExposureEnd
		// This time, the real-world simulation is notified to setup the next scene at an earlier point in time than before,
		// leading to a reduced cycle time.
		std::cout << "Test WITH EventExposureEnd" << std::endl;
		runTest(grabber, realWorld, 50, &eventExposureEnd);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}

#include <ic4/ic4.h>

#include <console-helper.h>


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
		 * This example shows how to receive events EventLine1RisingEdge and EventLine1FallingEdge, which indicate activity
		 * on one of the camera's digital inputs.
		 *
		 * Events are configured and received through the device property map.
		 * The following shows an excerpt from the device property map of a device supporting EventLine1RisingEdge and EventLine1FallingEdge:
		 *
		 * - EventControl
		 *   - EventSelector
		 *   - EventNotification[EventSelector]
		 *   - EventLine1RisingEdgeData
		 *     - EventLine1RisingEdge
		 *     - EventLine1RisingEdgeTimestamp
		 *   - EventLine1FallingEdgeData
		 *     - EventLine1FallingEdge
		 *     - EventLine1FallingEdgeTimestamp
		 *
		 * To receive notifications for a specific event, two steps have to be taken:
		 *
		 * First, the device has to be configured to send generate the specific event. To enable the EventLine1RisingEdge event, set the
		 * "EventSelector" enumeration property to "EventLine1RisingEdge", and then set the "EventNotification" enumeration property to "On".
		 *
		 * Second, a property notification handler has to be registered for the property representing the event.
		 * The EventLine1RisingEdge is represented by the integer property "EventLine1RisingEdge". This property has no function other
		 * than being invalidated and thus having its notification raised when the device sends the event.
		 *
		 * Event parameters are grouped with the event property in a property category with "Data" appended to the event's name,
		 * in our case "EventLine1RisingEdgeData". The category contains the integer property "EventLine1RisingEdgeTimestamp"
		 * which provides the time stamp of the event. Event argument properties should only be read inside the event notification
		 * function to avoid data races.
		 */

		 // Get Line1RisingEdge and Line1FallingEdge event properties
		auto eventLine1RisingEdge = grabber.devicePropertyMap().find(ic4::PropId::EventLine1RisingEdge);
		auto eventLine1FallingEdge = grabber.devicePropertyMap().find(ic4::PropId::EventLine1FallingEdge);
		// Get Line1RisingEdge and Line1FallingEdge timestamp arguments
		auto eventLine1RisingEdgeTimestamp = grabber.devicePropertyMap().find(ic4::PropId::EventLine1RisingEdgeTimestamp);
		auto eventLine1FallingEdgeTimestamp = grabber.devicePropertyMap().find(ic4::PropId::EventLine1FallingEdgeTimestamp);

		// Enable both Line1RisingEdge and Line1FallingEdge event notifications
		grabber.devicePropertyMap().setValue(ic4::PropId::EventSelector, "Line1RisingEdge");
		grabber.devicePropertyMap().setValue(ic4::PropId::EventNotification, "On");
		grabber.devicePropertyMap().setValue(ic4::PropId::EventSelector, "Line1FallingEdge");
		grabber.devicePropertyMap().setValue(ic4::PropId::EventNotification, "On");

		// Register notification for Line1RisingEdge
		auto risingEdgeToken = eventLine1RisingEdge.eventAddNotification(
			[&](ic4::Property&)
			{
				ic4::Error err; // Use error object to not throw from callback
				auto timestamp = eventLine1FallingEdgeTimestamp.getValue(err);
				if (err.isError())
				{
					std::cerr << "Failed to query EventLine1RisingEdgeTimestamp: " << err.message() << std::endl;
				}
				else
				{
					std::cout << "Line1 Rising Edge\t(Timestamp = " << timestamp << ")" << std::endl;
				}
			}
		);

		// Register notification for Line1FallingEdge
		auto fallingEdgeToken = eventLine1FallingEdge.eventAddNotification(
			[&](ic4::Property&)
			{
				ic4::Error err; // Use error object to not throw from callback
				auto timestamp = eventLine1FallingEdgeTimestamp.getValue(err);
				if (err.isError())
				{
					std::cerr << "Failed to query EventLine1FallingEdgeTimestamp: " << err.message() << std::endl;
				}
				else
				{
					std::cout << "Line1 Falling Edge\t(Timestamp = " << timestamp << ")" << std::endl;
				}
			}
		);

		std::cout << std::endl << "Waiting for Line1RisingEdge and Line1FallingEdge events. Press ENTER to exit." << std::endl;
		std::cin.get();

		// Unregister event notifications (for completeness only, we close the device anyway)
		eventLine1RisingEdge.eventRemoveNotification(risingEdgeToken);
		eventLine1FallingEdge.eventRemoveNotification(fallingEdgeToken);

		// Disable event notifications
		grabber.devicePropertyMap().setValue(ic4::PropId::EventSelector, "Line1RisingEdge");
		grabber.devicePropertyMap().setValue(ic4::PropId::EventNotification, "Off");
		grabber.devicePropertyMap().setValue(ic4::PropId::EventSelector, "Line1FallingEdge");
		grabber.devicePropertyMap().setValue(ic4::PropId::EventNotification, "Off");

		return 0;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An exception occurred: " << std::endl;
		std::cerr << ex.what() << std::endl;
		return -10;
	}
}
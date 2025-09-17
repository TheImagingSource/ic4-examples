
#include "stream_test.h"

#include <ic4/ic4.h>
#include <fmt/core.h>
#include <fmt/std.h>

#include <optional>
#include <string_view>
#include <thread>

#include "stream_test_camera.h"

namespace
{
	using cam_list = std::vector<std::unique_ptr<ic4ctrl::camera_instance>>;

	auto dump_stats_header() -> void
	{
		fmt::print("= {:^36} dev: {:>7}/{:>3}/{:>3}/{:>3} snk: {:>7}/{:>3}/{:>3} fps: {:>7.2f}",
			"device-name",
			"deliv", "err", "tr", "ur",     // dev
			"deliv", "ur", "ig",            // snk
			0.0                             // fps
		);
		fmt::print(" pckt: {:>5}", "res");
		fmt::print(" Mbps: {:>6}", 0);
		fmt::print("\n");
	}

	auto dump_stats(std::string_view dev_name, const ic4ctrl::camera_instance::stats& stats) -> void
	{
		if (stats.device_lost)
		{
			fmt::println("= {:^36} dev: {:>7}/{:>3}/{:>3}/{:>3} snk: {:>7}/{:>3}/{:>3} fps: {:>7} pckt: {:>5} Mbps: {:>6}",
				dev_name,
				stats.ic4stats_.device_delivered, stats.ic4stats_.device_transmission_error, stats.ic4stats_.device_transform_underrun, stats.ic4stats_.device_underrun,
				stats.ic4stats_.sink_delivered, stats.ic4stats_.sink_underrun, stats.ic4stats_.sink_ignored,
				"lost", stats.StreamResendRequestedPackets, "lost"
			);
		}
		else
		{
			fmt::println("= {:^36} dev: {:>7}/{:>3}/{:>3}/{:>3} snk: {:>7}/{:>3}/{:>3} fps: {:>7.2f} pckt: {:>5} Mbps: {:>6}",
				dev_name,
				stats.ic4stats_.device_delivered, stats.ic4stats_.device_transmission_error, stats.ic4stats_.device_transform_underrun, stats.ic4stats_.device_underrun,
				stats.ic4stats_.sink_delivered, stats.ic4stats_.sink_underrun, stats.ic4stats_.sink_ignored,
				stats.fps(), stats.StreamResendRequestedPackets, stats.calc_MBitsPerSeconds()
			);
		}
	}

	auto dump_full_list(const cam_list& list, std::chrono::steady_clock::time_point start_time) -> void
	{
		const auto now = std::chrono::steady_clock::now();

		const auto offset_time = std::chrono::duration_cast<std::chrono::seconds>(now - start_time);

#if __cpp_lib_chrono >= 201907L
		auto const local_time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
		fmt::print("[{}], Offset: {:%H:%M:%S}\n", local_time, offset_time);
#else
		fmt::print("Offset: {:%H:%M:%S}\n", offset_time);
#endif

		dump_stats_header();

		int64_t accu = 0;
		for (auto& e : list)
		{
			auto stats = e->fetch_run_statistics();

			dump_stats(e->name(), stats);

			accu += stats.calc_MBitsPerSeconds();
		}
		fmt::println("Sum of Gbit per second: {}", accu / 1000.f);
	}


	auto sleep_interval(std::chrono::milliseconds total_amount, bool print_state) -> void
	{
		if (total_amount < std::chrono::seconds(1)) {
			if (print_state) {
				fmt::println("Waiting {}ms", total_amount.count());
			}
			std::this_thread::sleep_for(total_amount);
			return;
		}

		std::chrono::milliseconds interval = std::chrono::seconds(1);
		if (print_state) {
			fmt::println("Waiting {}ms (ticks are 1 second) ", total_amount.count());
		}

		while (total_amount > std::chrono::milliseconds(10))
		{
			std::this_thread::sleep_for(interval);
			fmt::print(".");
			std::fflush(stdout);
			total_amount -= interval;
		}
		fmt::println("");
	}
}



auto ic4ctrl::start_stream_test(const stream_test_parameter& params, std::vector<ic4::DeviceInfo>& device_info_list) -> void
{
	std::vector<std::unique_ptr<ic4ctrl::camera_instance>> list;

	for (auto&& nfo : device_info_list) {
		
		try
		{
			auto cam = ic4ctrl::camera_instance::create(nfo);

			if (params.use_largest_stream_settings) {
				cam->adjust_videoformat();
			}
			cam->setup_stream();
			list.emplace_back(std::move(cam));
		}
		catch (const std::exception& ex)
		{
			// skip this here
		}
	}

	fmt::println("Stream stats list:");
	fmt::println("  dev: device_delivered/device_transmission_error/device_transform_underrun/device_underrun");
	fmt::println("  snk: sink_delivered/sink_underrun/sink_ignored");
	fmt::println("  pckt: res ^=  resend-packets-send");


	fmt::println("");
	fmt::println("Starting Streams for {} cameras.", list.size());

	auto start_time = std::chrono::steady_clock::now();

	for (auto& e : list)
	{
		e->start();
	}

	if (!params.once)
	{
		auto interval = params.stream_interval_in_seconds.value_or(5);

		fmt::println("Started all camera streams. Looping endlessly.");
		while (true)
		{
			sleep_interval(std::chrono::seconds(interval), false);

			fmt::println("");

			dump_full_list(list, start_time);

			fmt::println("");
		}
	}
	else
	{
		auto end_time = params.stream_interval_in_seconds.value_or(20);

		fmt::println("Started all camera streams. Streaming for {} seconds.", end_time);

		sleep_interval(std::chrono::seconds(end_time), false);
	}

	for (auto& e : list)
	{
		e->stop();
	}

	fmt::println("Stopped all camera streams.");
	fmt::println("");

	fmt::println("Device list:");
	for (auto& e : list)
	{
		fmt::println("= {:^27} {}", e->name(), e->vid_info());
	}
	fmt::println("\n");



	dump_full_list(list, start_time);
}

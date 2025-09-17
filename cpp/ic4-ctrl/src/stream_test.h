
#pragma once

#include <ic4/ic4.h>

#include <vector>
#include <optional>

namespace ic4ctrl
{
	struct stream_test_parameter
	{
		std::optional<unsigned int> stream_interval_in_seconds;
		bool once = false;
		bool use_largest_stream_settings = false;
	};

	auto start_stream_test(const stream_test_parameter& params, std::vector<ic4::DeviceInfo>& device_info_list) -> void;
}
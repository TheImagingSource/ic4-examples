#pragma once

#include "ic4-ctrl-helper.h"

#include <ic4/ic4.h>

#include <string>
#include <vector>

namespace helper
{
	auto read_IPAddressList(ic4::PropertyMap& map, ic4::PropInteger& subnet_ip, bool add_mask) -> std::vector<std::string>;
	auto print_interface_short(int offset, size_t id, ic4::Interface& itf) -> void;
	auto print_device_short(int offset, size_t index, ic4::DeviceInfo& dev, bool print_transport_layer) -> void;

	auto select_device_in_interface_DeviceSelector(const ic4::DeviceInfo& dev, ic4::PropertyMap& itf_prop_map) -> bool;
}
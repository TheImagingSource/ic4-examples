
#include "print_ic4_object.h"

#include "ic4_enum_to_string.h"

#include <fmt/std.h>
#include <fmt/ranges.h>
#include "print_property.h"

auto helper::read_IPAddressList(ic4::PropertyMap& map, ic4::PropInteger& subnet_ip, bool add_mask) -> std::vector<std::string>
{
	if (!subnet_ip.is_valid())	return {};

	auto selector = map.findInteger("GevInterfaceSubnetSelector", ic4::Error::Ignore());
	auto subnet_mask = map.findInteger("GevInterfaceSubnetMask", ic4::Error::Ignore());
	if (!subnet_mask.is_valid()) {
		add_mask = false;
	}

	std::vector<std::string> lst;
	try
	{

		if (!selector.is_valid()) {
			// throws on error
			return { helper::format_int_prop(subnet_ip.getValue(), ic4::PropIntRepresentation::IPV4Address) };
		}

		auto max_idx = selector.maximum();
		for (auto idx = selector.minimum(); idx <= max_idx; ++idx)
		{
			selector.setValue(idx);

			auto addr = subnet_ip.getValue();
			if (add_mask)
			{
				lst.push_back(fmt::format("{}/{}",
					helper::format_int_prop(addr, ic4::PropIntRepresentation::IPV4Address),
					helper::format_int_prop(subnet_mask.getValue(), ic4::PropIntRepresentation::IPV4Address)
				));
			}
			else
			{
				lst.push_back(
					helper::format_int_prop(addr, ic4::PropIntRepresentation::IPV4Address)
				);
			}
		}
		return lst;
	}
	catch (const std::exception& /*ex*/)
	{
	}
	return {};
}

auto helper::print_interface_short(int offset, size_t index, ic4::Interface& itf) -> void
{
	auto map = itf.interfacePropertyMap();

	std::string add_info;

	auto mtu = map.findInteger("MaximumTransmissionUnit", ic4::Error::Ignore());
	if (mtu.is_valid()) {
		add_info += fmt::format(" MTU={}", helper::get_value_as_string(mtu));
	}

	auto ipaddr = map.findInteger("GevInterfaceSubnetIPAddress", ic4::Error::Ignore());
	if (ipaddr.is_valid())
	{
		add_info += fmt::format(" IPv4={::}", read_IPAddressList(map, ipaddr, false));
	}

	print(offset, "{:^2} {:64}\n", index, itf.interfaceDisplayName());
	if (!add_info.empty()) {
		print(offset + 1, "{}\n", add_info);
	}
}

auto helper::print_device_short(int offset, size_t index, ic4::DeviceInfo& dev) -> void
{
	std::string add_info;

	print(offset, "{:^2} {:24} {:8} {:16}\n", index, 
		dev.modelName(ic4::Error::Ignore()), dev.serial(ic4::Error::Ignore()), dev.userID(ic4::Error::Ignore())
	);

	if (!add_info.empty()) {
		print(offset + 1, "{}\n", add_info);
	}
}


#include "helper_json.h"

#include "ic4_enum_to_string.h"

#include <fmt/ranges.h>
#include <nlohmann/json.hpp>
#include "print_ic4_object.h"

namespace
{
	template<class Tprop, class TMethod>
	auto add_json_line_(nlohmann::ordered_json& parent, const char* name, Tprop& prop, TMethod method_address) -> void
	{
		ic4::Error err;
		auto v = (prop.*method_address)(err);
		if (err.isError()) {
			if (err.code() == ic4::ErrorCode::GenICamNotImplemented) {
				parent[name] = "n/a";
			}
			else {
				parent[name] = "err";
			}
		}
		else {
			parent[name] = v;
		}
	}

	template<class TFunc>
	auto add_json_line_func(nlohmann::ordered_json& parent, const char* name, TFunc func) -> void
	{
		ic4::Error err;
		auto v = func(err);
		if (err.isError()) {
			if (err.code() == ic4::ErrorCode::GenICamNotImplemented) {
				parent[name] = "n/a";
			}
			else {
				parent[name] = "err";
			}
		}
		else {
			parent[name] = v;
		}
	}

	template<class T>
	auto add_json_line_(nlohmann::ordered_json& parent, const std::string& name, const T& val) -> void
	{
		parent[name] = val;
	}

	auto to_name_vec(const std::vector<ic4::Property>& lst)
	{
		std::vector<std::string> str;
		for (auto&& entry : lst) {
			str.push_back(entry.name());
		}
		return str;
	}

	auto to_string_vec(const nlohmann::ordered_json& arr) -> std::vector<std::string>
	{
		std::vector<std::string> str;
		for (auto&& entry : arr) {
			str.push_back(entry);
		}
		return str;
	}

	auto to_json_(const ic4::Property& property) -> nlohmann::ordered_json
	{
		using namespace ic4_helper;

		nlohmann::ordered_json rval;

		auto add_json_line = [&](const std::string& name, const auto& val) {
			rval[name] = val;
			};

		add_json_line("name", property.name());
		add_json_line("type", toString(property.type()));
		add_json_line("displayName", property.displayName());
		add_json_line("description", property.description());
		add_json_line("tooltip", property.tooltip());
		add_json_line("visibility", toString(property.visibility()));

		add_json_line("isAvailable", property.isAvailable());
		add_json_line("isLocked", property.isLocked());
		add_json_line("isReadOnly", property.isReadOnly());

		if (property.isSelector())
		{
			add_json_line("selectedProperties", to_name_vec(property.selectedProperties()));
		}

		switch (property.type())
		{
		case ic4::PropType::Integer:
		{
			ic4::PropInteger prop = property.asInteger();
			auto inc_mode = prop.incrementMode();
			auto rep = prop.representation();

			add_json_line("representation", toString(prop.representation()));
			add_json_line("unit", prop.unit());
			add_json_line("incrementMode", toString(prop.incrementMode()));

			if (prop.isAvailable())
			{
				if (inc_mode == ic4::PropIncrementMode::ValueSet)
				{
					ic4::Error err;
					const auto vvset = prop.validValueSet(err);
					if (!err.isError()) {
						add_json_line("validValueSet", vvset);
					}
				}
				else
				{
					if (!prop.isReadOnly())
					{
						add_json_line_(rval, "minimum", prop, &ic4::PropInteger::minimum);
						add_json_line_(rval, "maximum", prop, &ic4::PropInteger::maximum);

						if (inc_mode == ic4::PropIncrementMode::Increment)
						{
							add_json_line_(rval, "increment", prop, &ic4::PropInteger::increment);
						}
					}
				}
				add_json_line_(rval, "value", prop, &ic4::PropInteger::getValue);
			}
			break;
		}
		case ic4::PropType::Float:
		{
			ic4::PropFloat prop = property.asFloat();
			auto inc_mode = prop.incrementMode();

			add_json_line("representation", toString(prop.representation()));
			add_json_line("unit", prop.unit());
			add_json_line("incrementMode", toString(prop.incrementMode()));
			add_json_line("displayNotation", toString(prop.displayNotation()));
			add_json_line("displayPrecision", prop.displayPrecision());

			if (prop.isAvailable())
			{
				if (inc_mode == ic4::PropIncrementMode::ValueSet)
				{
					ic4::Error err;
					const auto vvset = prop.validValueSet(err);
					if (!err.isError()) {
						add_json_line("validValueSet", vvset);
					}
				}
				else
				{
					if (!prop.isReadOnly())
					{
						add_json_line_(rval, "minimum", prop, &ic4::PropFloat::minimum);
						add_json_line_(rval, "maximum", prop, &ic4::PropFloat::maximum);

						if (inc_mode == ic4::PropIncrementMode::Increment)
						{
							add_json_line_(rval, "increment", prop, &ic4::PropFloat::increment);
						}
					}
				}
				add_json_line_(rval, "value", prop, &ic4::PropFloat::getValue);
			}
			break;
		}
		case ic4::PropType::Enumeration:
		{
			auto prop = property.asEnumeration();

			std::vector<nlohmann::json> entries;
			for (auto&& entry : prop.entries(ic4::Error::Ignore()))
			{
				entries.push_back(to_json_(entry));
			}

			rval.push_back({ "entries", entries });

			if (prop.isAvailable())
			{
				add_json_line_(rval,"value", prop, &ic4::PropEnumeration::getValue);
			}
			break;
		}
		case ic4::PropType::Boolean:
		{
			auto prop = property.asBoolean();

			if (prop.isAvailable()) {
				add_json_line("value", prop.getValue());
			}
			break;
		}
		case ic4::PropType::String:
		{
			auto prop = property.asString();

			if (prop.isAvailable()) {
				add_json_line("value", prop.getValue());
				add_json_line("maxLength", prop.maxLength());
			}
			break;
		}
		case ic4::PropType::Command:
		{
			break;
		}
		case ic4::PropType::Category:
		{
			auto prop = property.asCategory();
			add_json_line("features", to_name_vec(prop.features(ic4::Error::Ignore())));
			break;
		}
		case ic4::PropType::Register:
		{
			ic4::PropRegister prop = property.asRegister();

			add_json_line("size", prop.size(ic4::Error::Ignore()));

			if (prop.isAvailable()) {
				add_json_line_(rval, "value", prop, &ic4::PropRegister::getValue);
			}
			break;
		}
		case ic4::PropType::Port:
		{
			break;
		}
		case ic4::PropType::EnumEntry:
		{
			ic4::PropEnumEntry prop = property.asEnumEntry();

			if (prop.isAvailable()) {
				add_json_line("intValue", prop.intValue());
			}
			break;
		}
		default:
			;
		};

		return rval;
	}

	auto add_json_from_map(nlohmann::ordered_json& rval, ic4::PropertyMap& map, const char* prop_name) -> void
	{
		ic4::Error err;
		auto contents = map.getValueString(prop_name, err);
		if (!err) {
			add_json_line_(rval, prop_name, contents);
		}
	}
}

auto helper::to_json_string(const ic4::Property& prop) -> std::string
{
	auto rval = to_json_(prop);
	return rval.dump(4);
}

auto helper::to_json_string(const ic4::PropertyMap& map) -> std::string
{
	nlohmann::ordered_json root;
	for (auto&& e : map.all()) {
		root.push_back(to_json_(e));
	}
	return root.dump(4);
}

auto helper::to_json_string(const std::vector<ic4::DeviceInfo>& lst) -> std::string
{
	nlohmann::ordered_json root;
	for (auto&& entry : lst) {
		root.push_back(to_json(entry));
	}
	return root.dump(4);
}


auto helper::to_json_string(const std::vector<ic4::Interface>& lst) -> std::string
{
	nlohmann::ordered_json root;
	for (auto&& entry : lst) {
		root.push_back(to_json(entry));
	}
	return root.dump(4);
}

auto helper::to_json(const ic4::Interface& itf) -> nlohmann::ordered_json
{
	nlohmann::ordered_json rval;

	add_json_line_(rval, "DisplayName", itf, &ic4::Interface::interfaceDisplayName);
	add_json_line_(rval, "TransportLayerName", itf, &ic4::Interface::transportLayerName);
	add_json_line_func(rval, "TransportLayerType", [itf](auto& err) { return ic4_helper::toString(itf.transportLayerType(err)); });
	add_json_line_(rval, "TransportLayerVersion", itf, &ic4::Interface::transportLayerVersion);

	auto map = itf.interfacePropertyMap();

	add_json_from_map(rval, map, "MaximumTransmissionUnit");

	auto ipaddr = map.findInteger("GevInterfaceSubnetIPAddress", ic4::Error::Ignore());
	if (ipaddr.is_valid())
	{
		add_json_line_(rval, "GevInterfaceSubnet", helper::read_IPAddressList(map, ipaddr, true));
	}

	return rval;
}

auto helper::to_json(const ic4::DeviceInfo& dev) -> nlohmann::ordered_json
{
	nlohmann::ordered_json rval;

	auto itf = dev.getInterface();

	auto itf_prop_map = itf.interfacePropertyMap();

	add_json_line_(rval, "ModelName", dev, &ic4::DeviceInfo::modelName);
	add_json_line_(rval, "UniqueName", dev, &ic4::DeviceInfo::uniqueName);
	add_json_line_(rval, "Serial", dev, &ic4::DeviceInfo::serial);
	add_json_line_(rval, "Version", dev, &ic4::DeviceInfo::version);
	add_json_line_(rval, "UserID", dev, &ic4::DeviceInfo::userID);
	add_json_line_(rval, "TransportLayerName", itf.transportLayerName());

	if (select_device_in_interface_DeviceSelector(dev, itf_prop_map))
	{
		add_json_from_map(rval, itf_prop_map, "DeviceReachableStatus");

		add_json_from_map(rval, itf_prop_map, "GevDeviceMACAddress");
		add_json_from_map(rval, itf_prop_map, "GevDeviceIPAddress");
		add_json_from_map(rval, itf_prop_map, "GevDeviceSubnetMask");
		add_json_from_map(rval, itf_prop_map, "GevDeviceGateway");
	}
	return rval;
}


auto print_json_line(int offset, const std::string& prop_name, const std::string& contents)
{
	print(offset, "{:32}: '{}'\n", prop_name, contents);
}

auto helper::print_json(int offset, const nlohmann::ordered_json& json) -> void
{
	for (auto&& entry : json.items())
	{
		auto key = entry.key();
		auto value = entry.value();
		if (value.is_array())
		{
			print_json_line(offset, entry.key(), fmt::format("{}", to_string_vec(value)));
		}
		else if (value.is_object())
		{
			print(offset, "{32}:", key);
			print_json(offset + 1, value);
		}
		else
		{
			print_json_line(offset, entry.key(), value);
		}
	}
}


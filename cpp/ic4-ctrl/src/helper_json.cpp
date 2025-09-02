
#include "helper_json.h"

#include "ic4_enum_to_string.h"

#include <nlohmann/json.hpp>

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

	auto to_json_(const ic4::DeviceInfo& dev) -> nlohmann::ordered_json
	{
		nlohmann::ordered_json rval;

		add_json_line_(rval, "modelName", dev, &ic4::DeviceInfo::modelName);
		add_json_line_(rval, "uniqueName", dev, &ic4::DeviceInfo::uniqueName);
		add_json_line_(rval, "serial", dev, &ic4::DeviceInfo::serial);
		add_json_line_(rval, "version", dev, &ic4::DeviceInfo::version);
		add_json_line_(rval, "userID", dev, &ic4::DeviceInfo::userID);

		return rval;
	}

	auto to_json_(const ic4::Interface& itf) -> nlohmann::ordered_json
	{
		nlohmann::ordered_json rval;

		add_json_line_(rval, "interfaceDisplayName", itf, &ic4::Interface::interfaceDisplayName);
		add_json_line_(rval, "transportLayerName", itf, &ic4::Interface::transportLayerName);
		add_json_line_func(rval, "transportLayerType", [itf](auto& err) { return ic4_helper::toString(itf.transportLayerType(err)); });
		add_json_line_(rval, "transportLayerVersion", itf, &ic4::Interface::transportLayerVersion);

		return rval;
	}
}

auto helper::to_json(const ic4::Property& prop) -> std::string
{
	auto rval = to_json_(prop);
	return rval.dump(4);
}

auto helper::to_json(const ic4::Interface& itf) -> std::string
{
	auto rval = to_json_(itf);
	return rval.dump(4);
}
auto helper::to_json(const ic4::DeviceInfo& dev) -> std::string
{
	auto rval = to_json_(dev);
	return rval.dump(4);
}

auto helper::to_json(const ic4::PropertyMap& map) -> std::string
{
	nlohmann::ordered_json root;
	for (auto&& e : map.all()) {
		root.push_back(to_json_(e));
	}
	return root.dump(4);
}

auto helper::to_json(const std::vector<ic4::DeviceInfo>& lst) -> std::string
{
	nlohmann::ordered_json root;
	for (auto&& entry : lst) {
		root.push_back(to_json_(entry));
	}
	return root.dump(4);
}


auto helper::to_json(const std::vector<ic4::Interface>& lst) -> std::string
{
	nlohmann::ordered_json root;
	for (auto&& entry : lst) {
		root.push_back(to_json_(entry));
	}
	return root.dump(4);
}
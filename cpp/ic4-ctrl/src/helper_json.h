
#pragma once

#include "ic4-ctrl-helper.h"

#include <ic4/ic4.h>

#include <nlohmann/json.hpp>

namespace helper
{
	auto	to_json_string(const ic4::Property& prop) -> std::string;
	auto	to_json_string(const ic4::PropertyMap& map) -> std::string;
	auto	to_json_string(const std::vector<ic4::DeviceInfo>& dev) -> std::string;
	auto	to_json_string(const std::vector<ic4::Interface>& itf) -> std::string;
	
	auto	to_json(const ic4::DeviceInfo& dev) -> nlohmann::ordered_json;
	auto	to_json(const ic4::Interface& dev) -> nlohmann::ordered_json;

	auto	print_json(int offset, const nlohmann::ordered_json& json) -> void;
}


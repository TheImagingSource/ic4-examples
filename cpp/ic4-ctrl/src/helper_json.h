
#pragma once

#include "ic4-ctrl-helper.h"

#include <ic4/ic4.h>

namespace helper
{
	auto	to_json(const ic4::Property& prop) -> std::string;
	auto	to_json(const ic4::PropertyMap& map) -> std::string;

	auto	to_json(const ic4::DeviceInfo& dev) -> std::string;
	auto	to_json(const std::vector<ic4::DeviceInfo>& dev) -> std::string;

	auto	to_json(const ic4::Interface& itf) -> std::string;
	auto	to_json(const std::vector<ic4::Interface>& itf) -> std::string;
}


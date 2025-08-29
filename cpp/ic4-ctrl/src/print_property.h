#pragma once

#include "ic4-ctrl-helper.h"

#include <ic4/ic4.h>

namespace helper
{
	auto	format_int_prop(int64_t v, ic4::PropIntRepresentation rep) -> std::string;

	auto	get_value_as_string(ic4::PropInteger& prop) -> std::string;
	void    print_property(int offset, const ic4::Property& property);
	void    print_property_short(int offset, const ic4::Property& property);
}
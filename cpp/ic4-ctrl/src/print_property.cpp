
#include "print_property.h"

#include "ic4_enum_to_string.h"

#include <fmt/std.h>
#include <fmt/ranges.h>

namespace
{
	template<class Tprop, class TMethod>
	auto fetch_PropertyMethod_value(Tprop& prop, TMethod method_address) -> std::string
	{
		ic4::Error err;
		auto v = (prop.*method_address)(err);
		if (err.isError()) {
			return "'err'";
		}
		return fmt::format("{}", v);
	}

	template<class TMethod>
	auto fetch_PropertyMethod_value(ic4::PropInteger& prop, TMethod method_address, ic4::PropIntRepresentation int_rep) -> std::string
	{
		ic4::Error err;
		int64_t v = (prop.*method_address)(err);
		if (err.isError()) {
			if (err.code() == ic4::ErrorCode::GenICamNotImplemented) {
				return "n/a";
			}
			return "err";
		}
		if (v == std::numeric_limits<int64_t>::lowest()) {
			return "MIN_INT";
		}
		if (v == std::numeric_limits<int64_t>::max()) {
			return "MAX_INT";
		}
		auto rval = helper::format_int_prop(v, int_rep);
		return rval;
	}

}

auto helper::format_int_prop(int64_t v, ic4::PropIntRepresentation rep) -> std::string
{
	switch (rep)
	{
	case ic4::PropIntRepresentation::Boolean:       return fmt::format("{}", v != 0 ? 1 : 0);
	case ic4::PropIntRepresentation::HexNumber:     return fmt::format("0x{:X}", v);
	case ic4::PropIntRepresentation::IPV4Address:
	{
		uint64_t v0 = (v >> 0) & 0xFF;
		uint64_t v1 = (v >> 8) & 0xFF;
		uint64_t v2 = (v >> 16) & 0xFF;
		uint64_t v3 = (v >> 24) & 0xFF;
		return fmt::format("{}.{}.{}.{}", v3, v2, v1, v0);
	}
	case ic4::PropIntRepresentation::MACAddress:
	{
		uint64_t v0 = (v >> 0) & 0xFF;
		uint64_t v1 = (v >> 8) & 0xFF;
		uint64_t v2 = (v >> 16) & 0xFF;
		uint64_t v3 = (v >> 24) & 0xFF;
		uint64_t v4 = (v >> 32) & 0xFF;
		uint64_t v5 = (v >> 40) & 0xFF;
		return fmt::format("{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", v5, v4, v3, v2, v1, v0);
	}
	case ic4::PropIntRepresentation::Linear:
	case ic4::PropIntRepresentation::Logarithmic:
	case ic4::PropIntRepresentation::PureNumber:
	default:
		return fmt::format("{}", v);
	}
}

auto helper::get_value_as_string(ic4::PropInteger& prop) -> std::string
{
	ic4::Error err;
	int64_t v = prop.getValue(err);
	if (err.isError()) {
		return {};
	}
	auto rep = prop.representation(err);
	if (err.isError()) {
		return {};
	}
	return format_int_prop(v, rep);
}

void helper::print_property(int offset, const ic4::Property& property)
{
	using namespace ic4_helper;

	auto prop_type = property.type();
	print(offset + 0, "{:24} - Type: {}, DisplayName: {}\n", property.name(), toString(prop_type), property.displayName());
	print(offset + 1, "Description: {}\n", property.description());
	print(offset + 1, "Tooltip: {}\n", property.tooltip());
	print(offset + 3, "\n");
	print(offset + 1, "Visibility: {}, Available: {}, Locked: {}, ReadOnly: {}\n", toString(property.visibility()), property.isAvailable(), property.isLocked(), property.isReadOnly());

	if (property.isSelector())
	{
		print(offset + 1, "Selected properties:\n");
		for (auto&& selected : property.selectedProperties())
		{
			print(offset + 2, "{}\n", selected.name());
		}
	}

	switch (prop_type)
	{
	case ic4::PropType::Integer:
	{
		ic4::PropInteger prop = property.asInteger();
		auto inc_mode = prop.incrementMode();
		auto rep = prop.representation();

		print(offset + 1, "Representation: '{}', Unit: '{}', IncrementMode: '{}'\n", toString(rep), prop.unit(), toString(inc_mode));

		if (prop.isAvailable())
		{
			if (!prop.isReadOnly()) {
				print(offset + 1, "Min: {}, Max: {}\n",
					fetch_PropertyMethod_value(prop, &ic4::PropInteger::minimum, rep),
					fetch_PropertyMethod_value(prop, &ic4::PropInteger::maximum, rep)
				);
			}
			if (inc_mode == ic4::PropIncrementMode::Increment) {
				if (!prop.isReadOnly())
				{
					print(offset + 1, "Inc: {}\n",
						fetch_PropertyMethod_value(prop, &ic4::PropInteger::increment, rep)
					);
				}
			}
			else if (inc_mode == ic4::PropIncrementMode::ValueSet)
			{
				ic4::Error err;
				std::vector<int64_t> vvset = prop.validValueSet(err);
				if (err.isError()) {
					print(offset + 1, "Failed to fetch ValidValueSet\n");
				}
				else
				{
					print(offset + 1, "ValidValueSet:\n");
					for (auto&& val : vvset) {
						print(offset + 2, "{}\n", val);
					}
					print("\n");
				}
			}
			print(offset + 1, "Value: {}\n",
				fetch_PropertyMethod_value(prop, &ic4::PropInteger::getValue, rep)
			);
		}
		break;
	}
	case ic4::PropType::Float:
	{
		ic4::PropFloat prop = property.asFloat();
		auto inc_mode = prop.incrementMode();

		print(offset + 1, "Representation: '{}', Unit: '{}', IncrementMode: '{}', DisplayNotation: {}, DisplayPrecision: {}\n",
			toString(prop.representation()), prop.unit(), toString(inc_mode), toString(prop.displayNotation()), prop.displayPrecision());

		if (prop.isAvailable())
		{
			if (!prop.isReadOnly()) {
				print(offset + 1, "Min: {}, Max: {}\n",
					fetch_PropertyMethod_value(prop, &ic4::PropFloat::minimum),
					fetch_PropertyMethod_value(prop, &ic4::PropFloat::maximum)
				);
			}

			if (inc_mode == ic4::PropIncrementMode::Increment) {
				print(offset + 1, "Inc: {}\n",
					fetch_PropertyMethod_value(prop, &ic4::PropFloat::increment)
				);
			}
			else if (inc_mode == ic4::PropIncrementMode::ValueSet)
			{
				ic4::Error err;
				std::vector<double> vvset = prop.validValueSet(err);
				if (err.isError()) {
					print(offset + 1, "Failed to fetch ValidValueSet\n");
				}
				else
				{
					print(offset + 1, "ValidValueSet:\n");
					for (auto&& val : vvset) {
						print(offset + 2, "{}\n", val);
					}
					print("\n");
				}
			}

			print(offset + 1, "Value: {}\n",
				fetch_PropertyMethod_value(prop, &ic4::PropFloat::getValue)
			);
		}
		break;
	}
	case ic4::PropType::Enumeration:
	{
		auto prop = property.asEnumeration();
		print(offset + 1, "EnumEntries:\n");
		for (auto&& entry : prop.entries(ic4::Error::Ignore()))
		{
			auto prop_enum_entry = entry.asEnumEntry();

			print_property(offset + 2, prop_enum_entry);
			print("\n");
		}

		if (prop.isAvailable())
		{
			ic4::Error err;
			auto selected_entry = prop.selectedEntry(err);
			if (err) {
				print(offset + 1, "Value: {}, SelectedEntry.Name: '{}'\n", "err", "err");
			}
			else
			{
				print(offset + 1, "Value: {}, SelectedEntry.Name: '{}'\n",
					fetch_PropertyMethod_value(prop, &ic4::PropEnumeration::getIntValue),
					selected_entry.name()
				);
			}
		}
		break;
	}
	case ic4::PropType::Boolean:
	{
		auto prop = property.asBoolean();

		if (prop.isAvailable()) {
			print(offset + 1, "Value: {}\n",
				fetch_PropertyMethod_value(prop, &ic4::PropBoolean::getValue)
			);
		}
		break;
	}
	case ic4::PropType::String:
	{
		auto prop = property.asString();

		if (prop.isAvailable()) {
			print(offset + 1, "Value: '{}', MaxLength: {}\n",
				fetch_PropertyMethod_value(prop, &ic4::PropString::getValue),
				fetch_PropertyMethod_value(prop, &ic4::PropString::maxLength)
			);
		}
		break;
	}
	case ic4::PropType::Command:
	{
		print("\n");
		break;
	}
	case ic4::PropType::Category:
	{
		auto prop = property.asCategory();
		print(offset + 1, "Features:\n");
		for (auto&& feature : prop.features(ic4::Error::Ignore()))
		{
			print(offset + 2, "{}\n", feature.name());
		}
		break;
	}
	case ic4::PropType::Register:
	{
		ic4::PropRegister prop = property.asRegister();

		print(offset + 1, "Size: {}\n",
			fetch_PropertyMethod_value(prop, &ic4::PropRegister::size)
		);
		if (prop.isAvailable()) {
			ic4::Error err;
			std::vector<uint8_t> vec = prop.getValue(err);
			if (err.isError()) {
				print(offset + 1, "Value: 'err'");
			}
			else {
				std::string str;
				size_t max_entries_to_print = 16;
				for (size_t i = 0; i < std::min(max_entries_to_print, vec.size()); ++i) {
					str += fmt::format("{:x}", vec[i]);
					str += ", ";
				}
				if (vec.size() > max_entries_to_print) {
					str += "...";
				}
				print(offset + 1, "Value: [{}], Value-Size: {}\n", str, vec.size());
			}
		}
		print("\n");
		break;
	}
	case ic4::PropType::Port:
	{
		print("\n");
		break;
	}
	case ic4::PropType::EnumEntry:
	{
		ic4::PropEnumEntry prop = property.asEnumEntry();

		if (prop.isAvailable()) {
			print(offset + 1, "IntValue: {}\n", fetch_PropertyMethod_value(prop, &ic4::PropEnumEntry::intValue));
		}
		print("\n");
		break;
	}
	default:
		;
	};
	print("\n");
}

void helper::print_property_short(int offset, const ic4::Property& property)
{
	using namespace ic4_helper;

	auto prop_type = property.type();
	print(offset + 0, "{} - {}, Av: {}, Lck: {}, RO: {} ", 
		property.name(), 
		toString(property.type()),
		property.isAvailable() ? 1 : 0, property.isLocked() ? 1 : 0, property.isReadOnly() ? 1 : 0);

	switch (prop_type)
	{
	case ic4::PropType::Integer:
	{
		ic4::PropInteger prop = property.asInteger();
		auto inc_mode = prop.incrementMode();
		auto rep = prop.representation();

		if (prop.isAvailable())
		{
			std::string range_string;
			if (inc_mode == ic4::PropIncrementMode::ValueSet)
			{
				range_string = fmt::format(", Range: {}",
					fetch_PropertyMethod_value(prop, &ic4::PropInteger::validValueSet)
				);
			}
			else
			{
				if( rep == ic4::PropIntRepresentation::IPV4Address || rep == ic4::PropIntRepresentation::MACAddress)
				{
				}
				else
				{
					if (!prop.isReadOnly())
					{
						range_string += fmt::format("[{};{}",
							fetch_PropertyMethod_value(prop, &ic4::PropInteger::minimum, rep),
							fetch_PropertyMethod_value(prop, &ic4::PropInteger::maximum, rep)
						);
						if (inc_mode == ic4::PropIncrementMode::Increment)
						{
							range_string += fmt::format(";{}", fetch_PropertyMethod_value(prop, &ic4::PropInteger::increment));
						}
						range_string += ']';
					}
				}
			}

			print("Value: {} {} {}",
				fetch_PropertyMethod_value(prop, &ic4::PropInteger::getValue),
				prop.unit(),
				range_string
			);
		}
		else
		{
			print("Value: 'na'");
		}
		break;
	}
	case ic4::PropType::Float:
	{
		ic4::PropFloat prop = property.asFloat();
		auto inc_mode = prop.incrementMode();

		if (prop.isAvailable())
		{
			std::string range_string;
			if (inc_mode == ic4::PropIncrementMode::ValueSet)
			{
				range_string = fmt::format("{}",
					fetch_PropertyMethod_value(prop, &ic4::PropFloat::validValueSet)
					);
			}
			else
			{
				if (!prop.isReadOnly()) {
					range_string += fmt::format("[{};{}",
						fetch_PropertyMethod_value(prop, &ic4::PropFloat::minimum),
						fetch_PropertyMethod_value(prop, &ic4::PropFloat::maximum)
					);
					if (inc_mode == ic4::PropIncrementMode::Increment)
					{
						range_string += fmt::format(";{}", fetch_PropertyMethod_value(prop, &ic4::PropFloat::increment));
					}
					range_string += ']';
				}
			}

			print("Value: {} {} {}",
				fetch_PropertyMethod_value(prop, &ic4::PropFloat::getValue),
				prop.unit(),
				range_string
			);
		}
		else
		{
			print("Value: 'na'");
		}
		break;
	}
	case ic4::PropType::Enumeration:
	{
		auto prop = property.asEnumeration();

		std::vector<std::string> entries;
		for (auto&& entry : prop.entries(ic4::Error::Ignore()))
		{
			entries.push_back(entry.name());
		}

		if (prop.isAvailable()) {
			print("Value: '{}' ({}), Entries: {}",
				fetch_PropertyMethod_value(prop, &ic4::PropEnumeration::getValue),
				fetch_PropertyMethod_value(prop, &ic4::PropEnumeration::getIntValue),
				entries
			);
		}
		else
		{
			print("Value: 'na', Entries: {}", entries);
		}
		break;
	}
	case ic4::PropType::Boolean:
	{
		auto prop = property.asBoolean();

		if (prop.isAvailable()) {
			print("Value: {}",
				fetch_PropertyMethod_value(prop, &ic4::PropBoolean::getValue)
			);
		}
		else
		{
			print("Value: 'na'");
		}
		break;
	}
	case ic4::PropType::String:
	{
		auto prop = property.asString();

		print("Value: '{}', MaxLength: {}",
			fetch_PropertyMethod_value(prop, &ic4::PropString::getValue),
			fetch_PropertyMethod_value(prop, &ic4::PropString::maxLength)
		);
		break;
	}
	case ic4::PropType::Command:
	{
		break;
	}
	case ic4::PropType::Category:
	{
		auto prop = property.asCategory();

		std::vector<std::string> arr;
		for (auto&& feature : prop.features(ic4::Error::Ignore()))
		{
			arr.push_back(feature.name());
		}
		print("Features: {}", arr);
		break;
	}
	case ic4::PropType::Register:
	{
		ic4::PropRegister prop = property.asRegister();

		const auto reg_size = fetch_PropertyMethod_value(prop, &ic4::PropRegister::size);
		std::string value_string;
		if (prop.isAvailable())
		{
			ic4::Error val_err;
			auto vec = prop.getValue(val_err);
			if (val_err.isError()) {
				value_string = "'err'";
			} else {
				size_t max_entries_to_print = 16;
				std::string str;
				if (vec.size() > 16) {
					vec.resize(16);
					str = " ...";
				}
				value_string = fmt::format("[{:n:02x}{}]", vec, str);
			}
		}
		else
		{
			value_string = "'na'";
		}
		print("Size: {} Value: {}", reg_size, value_string);
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
			print("IntValue: {}", fetch_PropertyMethod_value(prop, &ic4::PropEnumEntry::intValue));
		} else {
			print("IntValue: 'na'");
		}
		break;
	}
	default:
		;
	};
	print("\n\n");
}

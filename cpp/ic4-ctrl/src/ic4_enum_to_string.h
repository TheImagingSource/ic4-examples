#pragma once

namespace ic4_helper
{
    inline const char* toString( ic4::PropType type ) noexcept
    {
        switch( type )
        {
        case ic4::PropType::Invalid:        return "Invalid";
        case ic4::PropType::Integer:        return "Integer";
        case ic4::PropType::Float:          return "Float";
        case ic4::PropType::Enumeration:    return "Enumeration";
        case ic4::PropType::Boolean:        return "Boolean";
        case ic4::PropType::String:         return "String";
        case ic4::PropType::Command:        return "Command";
        case ic4::PropType::Category:       return "Category";
        case ic4::PropType::Register:       return "Register";
        case ic4::PropType::Port:           return "Port";
        case ic4::PropType::EnumEntry:      return "EnumEntry";
        default:
            return "";
        }
    }

    inline const char* toString( ic4::PropVisibility val ) noexcept
    {
        switch( val )
        {
        case ic4::PropVisibility::Beginner:     return "Beginner";
        case ic4::PropVisibility::Expert:       return "Expert";
        case ic4::PropVisibility::Guru:         return "Guru";
        case ic4::PropVisibility::Invisible:    return "Invisible";
        default:
            return "";
        }
    }

    inline const char* toString( ic4::PropIntRepresentation val ) noexcept
    {
        switch( val )
        {
        case ic4::PropIntRepresentation::Linear:        return "Linear";
        case ic4::PropIntRepresentation::Logarithmic:   return "Logarithmic";
        case ic4::PropIntRepresentation::Boolean:       return "Boolean";
        case ic4::PropIntRepresentation::PureNumber:    return "PureNumber";
        case ic4::PropIntRepresentation::HexNumber:     return "HexNumber";
        case ic4::PropIntRepresentation::IPV4Address:   return "IPV4Address";
        case ic4::PropIntRepresentation::MACAddress:    return "MACAddress";
        default:
            return "";
        }
    }

    inline const char* toString( ic4::PropFloatRepresentation val ) noexcept
    {
        switch( val )
        {
        case ic4::PropFloatRepresentation::Linear:      return "Linear";
        case ic4::PropFloatRepresentation::Logarithmic: return "Logarithmic";
        case ic4::PropFloatRepresentation::PureNumber:  return "PureNumber";
        default:
            return "";
        }
    }

    inline const char* toString( ic4::PropDisplayNotation val ) noexcept
    {
        switch( val )
        {
        case ic4::PropDisplayNotation::Automatic:   return "Automatic";
        case ic4::PropDisplayNotation::Fixed:       return "Fixed";
        case ic4::PropDisplayNotation::Scientific:  return "Scientific";
        default:
            return "";
        }
    }


    inline const char* toString( ic4::PropIncrementMode val ) noexcept
    {
        switch( val )
        {
        case ic4::PropIncrementMode::Increment:     return "Incrmeent";
        case ic4::PropIncrementMode::ValueSet:      return "valueSet";
        case ic4::PropIncrementMode::None:          return "None";
        default:
            return "";
        }
    }
}
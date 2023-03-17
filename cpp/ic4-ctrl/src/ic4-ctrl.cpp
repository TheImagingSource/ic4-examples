
#include <ic4/ic4.h>

#include <CLI/CLI.hpp>
#include <fmt/core.h>

#include <condition_variable>
#include <mutex>
#include <string>

#include "ic4_enum_to_string.h"
#include "ic4-ctrl-helper.h"

void    print_property( int offset, const ic4::Property& property );

template<class ... Targs>
void print( fmt::format_string<Targs...> fmt, Targs&& ... args )
{
    fmt::print( fmt, std::forward<Targs>( args )... );
}

template<class ... Targs>
void print( int offset, fmt::format_string<Targs...> fmt, Targs&& ... args )
{
    for( int i = 0; i < offset; ++i ) {
        fmt::print( "\t" );
    }
    fmt::print( fmt, std::forward<Targs>( args )... );
}

static auto find_device( std::string id ) -> std::unique_ptr<ic4::DeviceInfo>
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.getAvailableVideoCaptureDevices();
    if( list.size() == 0 ) {
        throw std::runtime_error( "No devices are available" );
    }

    for( auto&& dev : list )
    {
        if( dev.getSerial() == id ) {
            return std::make_unique<ic4::DeviceInfo>( dev );
        }
    }
    for( auto&& dev : list )
    {
        if( dev.getUniqueName() == id ) {
            return std::make_unique<ic4::DeviceInfo>( dev );
        }
    }
    for( auto&& dev : list )
    {
        if( dev.getModelName() == id ) {
            return std::make_unique<ic4::DeviceInfo>( dev );
        }
    }
    int64_t index = 0;
    if( helper::from_chars_helper( id, index ) )
    {
        if( index < 0 || index >= static_cast<int64_t>(list.size()) )
            return {};

        return std::make_unique<ic4::DeviceInfo>( list.at( index ) );
    }
    return {};
}

static auto find_inteface( std::string id ) -> std::unique_ptr<ic4::Interface>
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.getAvailableInterfaces();
    if( list.size() == 0 ) {
        throw std::runtime_error( "No devices are available" );
    }

    for( auto&& dev : list )
    {
        if( dev.getName() == id ) {
            return std::make_unique<ic4::Interface>( dev );
        }
    }
    for( auto&& dev : list )
    {
        if( dev.getTransportLayerName() == id ) {
            return std::make_unique<ic4::Interface>( dev );
        }
    }
    int64_t index = 0;
    if( helper::from_chars_helper( id, index ) )
    {
        if( index < 0 || index >= static_cast<int64_t>(list.size()) )
            return {};

        return std::make_unique<ic4::Interface>( list.at( index ) );
    }
    return {};
}


static auto list_devices() -> void
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.getAvailableVideoCaptureDevices();

    print( "Device list:\n" );
    print( "    {:24} {:8} {}\n", "ModelName", "Serial", "InterfaceName" );
    int index = 0;
    for( auto&& e : list ) {
        print( "{:>3} {:24} {:8} {}\n", index, e.getModelName(), e.getSerial(), e.getInterface().getTransportLayerName() );
    }
    if( list.empty() ) {
        print( "    No devices found\n" );
    }
}

static auto list_interfaces() -> void
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.getAvailableInterfaces( ic4::throwError );

    print( "Interface list:\n" );

    for( auto&& e : list ) {
        print( 1, "{}\n", e.getName() );
        print( 2, "TransportLayerName: {}\n", e.getTransportLayerName() );
    }
    if( list.empty() ) {
        print( 1, "No Interfaces found\n" );
    }
}

static void print_device( std::string id )
{
    auto dev = find_device( id );
    if( !dev ) {
        throw std::runtime_error( fmt::format( "Failed to find device for id '{}'\n", id ) );
    }

    print( "ModelName: '{}'\n", dev->getModelName() );
    print( "Serial: '{}'\n", dev->getSerial() );
    print( "UniqueName: '{}'\n", dev->getUniqueName() );
    print( "getDeviceVersion: '{}'\n", dev->getDeviceVersion() );
    print( "InterfaceName: '{}'\n", dev->getInterface().getTransportLayerName() );
}

static void print_interface( std::string id )
{
    auto dev = find_inteface( id );
    if( !dev ) {
        throw std::runtime_error( fmt::format( "Failed to find device for id '{}'\n", id ) );
    }

    print( "Name: '{}'\n", dev->getName() );
    print( "TransportLayerName: '{}'\n", dev->getTransportLayerName() );
    print( "TransportLayerType: '{}'\n", ic4_helper::toString( dev->getTransportLayerType() ) );
    print( "getTransportVersion: '{}'\n", dev->getTransportVersion() );
    
    print( "Interface Properties:\n" );
    auto map = dev->itfPropertyMap( ic4::throwError );
    for( auto&& property : map.getAll( ic4::throwError ) )
    {
        print_property( 1, property );
    }
}

template<typename TPropType, class Tprop, class TMethod>
auto fetch_PropertyMethod_value( Tprop& prop, TMethod method_address ) -> std::string
{
    TPropType v;
    ic4::Error err;
    if( !(prop.*method_address)(v, err) ) {
        if( err.getValue() == ic4::ErrorCode::GenICamNotImplemented ) {
            return "n/a";
        }
        return "err";
    }
    return fmt::format( "{}", v );
}

template<class TMethod>
auto fetch_PropertyMethod_value( ic4::PropInteger& prop, TMethod method_address, ic4::PropIntRepresentation int_rep ) -> std::string
{
    int64_t v;
    ic4::Error err;
    if( !(prop.*method_address)(v, err) ) {
        if( err.getValue() == ic4::ErrorCode::GenICamNotImplemented ) {
            return "n/a";
        }
        return "err";
    }
    if( int_rep == ic4::PropIntRepresentation::MACAddress )
    {
        uint64_t v0 = (v >> 0) & 0xFF;
        uint64_t v1 = (v >> 8) & 0xFF;
        uint64_t v2 = (v >> 16) & 0xFF;
        uint64_t v3 = (v >> 24) & 0xFF;
        uint64_t v4 = (v >> 32) & 0xFF;
        uint64_t v5 = (v >> 40) & 0xFF;
        return fmt::format( "{:02X}:{:02X}:{:02X}:{:02X}:{:02X}:{:02X}", v5, v4, v3, v2, v1, v0 );
    }
    else if( int_rep == ic4::PropIntRepresentation::IPV4Address )
    {
        uint64_t v0 = (v >> 0) & 0xFF;
        uint64_t v1 = (v >> 8) & 0xFF;
        uint64_t v2 = (v >> 16) & 0xFF;
        uint64_t v3 = (v >> 24) & 0xFF;
        return fmt::format( "{}.{}.{}.{}", v3, v2, v1, v0 );
    }
    else if( int_rep == ic4::PropIntRepresentation::HexNumber )
    {
        return fmt::format( "0x{:X}", v );
    }
    else if( int_rep == ic4::PropIntRepresentation::Boolean )
    {
        return fmt::format( "{}", v != 0 ? 1 : 0 );
    }
    return fmt::format( "{}", v );
}

static void    print_property( int offset, const ic4::Property& property )
{
    using namespace ic4_helper;

    auto prop_type = property.getType();
    print( offset + 0, "{:24} - Type: {}, DisplayName: {}\n", property.getName(), toString( prop_type ), property.getDisplayName() );
    print( offset + 1, "Description: {}\n", property.getDescription() );
    print( offset + 1, "Tooltip: {}\n", property.getTooltip() );
    print( offset + 1, "Visibility: {}, Available: {}, Locked: {}, ReadOnly: {}\n", toString( property.getVisibility() ), property.isAvailable(), property.isLocked(), property.isReadOnly() );

    if( property.isSelector( ic4::throwError ) )
    {
        print( offset + 1, "Selected properties:\n" );
        for( auto&& selected : property.getSelectedProperties( ic4::throwError ) )
        {
            print( offset + 2, "{}\n", selected.getName() );
        }
    }

    switch( prop_type )
    {
    case ic4::PropType::Integer:
    {
        ic4::PropInteger prop = property.asInteger();
        auto inc_mode = prop.getIncrementMode();
        auto rep = prop.getRepresentation();

        print( offset + 1, "Representation: '{}', Unit: '{}', IncrementMode: '{}'\n", toString( rep ), prop.getUnit(), toString( inc_mode ) );

        if( prop.isAvailable() )
        {
            if( !prop.isReadOnly() ) {
                print( offset + 1, "Min: {}, Max: {}\n",
                    fetch_PropertyMethod_value( prop, &ic4::PropInteger::getMinimum, rep ),
                    fetch_PropertyMethod_value( prop, &ic4::PropInteger::getMaximum, rep )
                );
            }
            if( inc_mode == ic4::PropIncrementMode::Increment ) {
                if( !prop.isReadOnly() )
                {
                    print( offset + 1, "Inc: {}\n",
                        fetch_PropertyMethod_value( prop, &ic4::PropInteger::getIncrement, rep )
                    );
                }
            }
            else if( inc_mode == ic4::PropIncrementMode::ValueSet )
            {
                std::vector<int64_t> vvset;
                if( !prop.getValidValueSet( vvset, ic4::ignoreError ) ) {
                    print( offset + 1, "Failed to fetch ValidValueSet\n" );
                }
                else
                {
                    print( offset + 1, "ValidValueSet:" );
                    for( auto&& val : vvset ) {
                        print( offset + 2, "{}\n", val );
                    }
                    print( "\n" );
                }
            }
            print( offset + 1, "Value: {}\n",
                fetch_PropertyMethod_value( prop, &ic4::PropInteger::getValue, rep )
            );
        }
        break;
    }
    case ic4::PropType::Float:
    {
        ic4::PropFloat prop = property.asFloat();
        auto inc_mode = prop.getIncrementMode();

        print( offset + 1, "Representation: '{}', Unit: '{}', IncrementMode: '{}', DisplayNotation: {}, DisplayPrecision: {}\n",
            toString( prop.getRepresentation() ), prop.getUnit(), toString( inc_mode ), toString( prop.getDisplayNotation() ), prop.getDisplayPrecision() );

        if( prop.isAvailable() )
        {
            if( !prop.isReadOnly() ) {
                print( offset + 1, "Min: {}, Max: {}\n",
                    fetch_PropertyMethod_value<double>( prop, &ic4::PropFloat::getMinimum ),
                    fetch_PropertyMethod_value<double>( prop, &ic4::PropFloat::getMaximum )
                );
            }

            if( inc_mode == ic4::PropIncrementMode::Increment ) {
                print( offset + 1, "Inc: {}\n",
                    fetch_PropertyMethod_value<double>( prop, &ic4::PropFloat::getIncrement )
                );
            }
            else if( inc_mode == ic4::PropIncrementMode::ValueSet )
            {
                //std::vector<double> vvset;
                //if( !prop.getValidValueSet( vvset, ic4::ignoreError ) ) {
                //    print( offset + 1, "Failed to fetch ValidValueSet\n" );
                //}
                //else
                //{
                //    print( offset + 1, "ValidValueSet:" );
                //    for( auto&& val : vvset ) {
                //        print( offset + 2, "{}\n", val );
                //    }
                //    print( "\n" );
                //}
            }

            print( offset + 1, "Value: {}\n",
                fetch_PropertyMethod_value<double>( prop, &ic4::PropFloat::getValue )
            );
        }
        break;
    }
    case ic4::PropType::Enumeration:
    {
        auto prop = property.asEnumeration();
        print( offset + 1, "EnumEntries:\n" );
        for( auto&& entry : prop.getEntries( ic4::ignoreError ) )
        {
            auto prop_enum_entry = entry.asEnumEntry();

            print( offset + 2, "{}, DisplayName: {}\n", prop_enum_entry.getName(), prop_enum_entry.getDisplayName() );
            print( offset + 3, "Description: {}\n", prop_enum_entry.getDescription() );
            print( offset + 3, "Tooltip: {}\n", prop_enum_entry.getTooltip() );
            print( offset + 3, "Visibility: {}, Available: {}, Locked: {}, ReadOnly: {}\n",
                toString( prop_enum_entry.getVisibility() ), prop_enum_entry.isAvailable(), prop_enum_entry.isLocked(), prop_enum_entry.isReadOnly() );

            if( prop_enum_entry.isAvailable() ) {
                print( offset + 3, "Value: {}\n", fetch_PropertyMethod_value<int64_t>( prop_enum_entry, &ic4::PropEnumEntry::getValue ) );
            }
            print( "\n" );
        }

        if( prop.isAvailable() )
        {
            ic4::Error err;
            auto selected_entry = prop.getSelectedEntry( err );
            if( err ) {
                print( offset + 1, "Value: {}, SelectedEntry.Name: '{}'\n", "err", "err" );
            }
            else
            {
                print( offset + 1, "Value: {}, SelectedEntry.Name: '{}'\n",
                    fetch_PropertyMethod_value<int64_t>( prop, &ic4::PropEnumeration::getValue ),
                    selected_entry.getName()
                );
            }
        }
        break;
    }
    case ic4::PropType::Boolean:
    {
        auto prop = property.asBoolean();

        if( prop.isAvailable() ) {
            print( offset + 1, "Value: {}\n",
                fetch_PropertyMethod_value<bool>( prop, &ic4::PropBoolean::getValue )
            );
        }
        break;
    }
    case ic4::PropType::String:
    {
        auto prop = property.asString();

        if( prop.isAvailable() ) {
            print( offset + 1, "Value: '{}', MaxLength: {}\n",
                fetch_PropertyMethod_value<std::string>( prop, &ic4::PropString::getValue ),
                fetch_PropertyMethod_value<uint64_t>( prop, &ic4::PropString::getMaxLength )
            );
        }
        break;
    }
    case ic4::PropType::Command:
    {
        print( "\n" );
        break;
    }
    case ic4::PropType::Category:
    {
        auto prop = property.asCategory();
        print( offset + 1, "Features:\n" );
        for( auto&& feature : prop.getFeatures( ic4::ignoreError ) )
        {
            print( offset + 2, "{}\n", feature.getName() );
        }
        break;
    }
    case ic4::PropType::Register:
    {
        ic4::PropRegister prop = property.asRegister();

        print( offset + 1, "Size: {}", 
            fetch_PropertyMethod_value<uint64_t>( prop, &ic4::PropRegister::getSize )
        );
        if( prop.isAvailable() ) {
            std::vector<uint8_t> vec;
            if( prop.getValue( vec, ic4::ignoreError ) ) {
                print( offset + 1, "Value: 'err'" );
            }
            else {
                std::string str;
                size_t max_entries_to_print = 16;
                for( size_t i = 0; i < std::min( max_entries_to_print, vec.size() ); ++i ) {
                    str += fmt::format( "{:x}", vec[i] );
                    str += ", ";
                }
                if( vec.size() > max_entries_to_print ) {
                    str += "...";
                }
                print( offset + 1, "Value: [{}], Value-Size: {}\n", str, vec.size() );
            }
        }
        print( "\n" );
        break;
    }
    case ic4::PropType::Port:
    {
        print( "\n" );
        break;
    }
    case ic4::PropType::EnumEntry:
    {
        print( "\n" );
        break;
    }
    default:
        ;
    };
    print( "\n" );
}

static void print_properties( std::string id, std::vector<std::string> lst )
{
    auto dev = find_device( id );
    if( !dev ) {
        print( "Failed to find device for id '{}'", id );
        return;
    }
    ic4::Grabber g;
    g.deviceOpen( *dev, ic4::throwError );

    auto map = g.devicePropertyMap();

    if( lst.empty() )
    {
        for( auto&& property : map.getAll( ic4::throwError ) )
        {
            print_property( 0, property );
        }
    }
    else
    {
        for( auto&& entry : lst )
        {
            auto property = map.get( entry.c_str() );
            if( property.is_valid() ) {
                print_property( 0, property );
            } else {
                print( "Failed to find property for name: '{}'\n", entry );
            }
        }
    }
}

static void save_properties( std::string id, std::string filename )
{
    auto dev = find_device( id );
    if( !dev ) {
        print( "Failed to find device for id '{}'", id );
        return;
    }
    ic4::Grabber g;
    g.deviceOpen( *dev, ic4::throwError );

    g.devicePropertyMap().serialize( filename, ic4::throwError );
}

static void save_image( std::string id, std::string filename, int count, int timeout_in_ms, std::string image_type )
{
    auto dev = find_device( id );
    if( !dev ) {
        print( "Failed to find device for id '{}'", id );
        return;
    }
    ic4::Grabber g;
    g.deviceOpen( *dev, ic4::throwError );

    auto snap_sink = ic4::SnapSink::create( ic4::throwError );
    g.streamSetup( snap_sink, ic4::StreamSetupOption::AcquisitionStart, ic4::throwError );

    ic4::Error err;
    auto images = snap_sink->snapSequence( count, timeout_in_ms, err );
    if( err ) {
        if( err.getValue() == ic4::ErrorCode::Timeout ) {
            print( "Timeout elapsed." );
            // #TODO maybe dissect what to do here.
            return;
        }
        throw err;
    }

    g.acquisitionStop( ic4::throwError );

    int idx = 0;
    for( auto && image : images )
    {
        std::string actual_filename = filename;
        if( filename.find_first_of( '{' ) != std::string::npos
            && filename.find_first_of( '}' ) != std::string::npos )
        {
            actual_filename = fmt::vformat( filename, fmt::make_format_args( idx++ ) );
        }
        if( image_type == "bmp" ) {
            ic4::imageBufferSaveAsBitmap( *image, actual_filename, {}, ic4::throwError );
        }
        else if( image_type == "png" ) {
            ic4::imageBufferSaveAsPng( *image, actual_filename, {}, ic4::throwError );
        }
        else if( image_type == "tiff" ) {
            ic4::imageBufferSaveAsTiff( *image, actual_filename, {}, ic4::throwError );
        }
        else if( image_type == "jpeg" ) {
            ic4::imageBufferSaveAsJpeg( *image, actual_filename, {}, ic4::throwError );
        }
    }
}


static void show_live( std::string id )
{
    auto dev = find_device( id );
    if( !dev ) {
        print( "Failed to find device for id '{}'", id );
        return;
    }
    ic4::Grabber g;
    g.deviceOpen( *dev, ic4::throwError );

    auto display = ic4::Display::create( ic4::DisplayType::Default, nullptr, ic4::throwError );
    g.streamSetup( display, ic4::StreamSetupOption::AcquisitionStart, ic4::throwError );

    std::mutex mtx;
    bool ended = false;
    std::condition_variable cond;

    display->eventAddWindowClosed( [&]( auto& ) { std::lock_guard<std::mutex> lck{ mtx }; ended = true; cond.notify_all(); } );

    {
        std::unique_lock<std::mutex> lck{ mtx };
        while( !ended ) {
            cond.wait( lck );
        }
    }
    g.acquisitionStop( ic4::throwError );
}

static void set_props( std::string id, std::vector<std::string> lst )
{
    auto dev = find_device( id );
    if( !dev ) {
        print( "Failed to find device for id '{}'", id );
        return;
    }
    ic4::Grabber g;
    g.deviceOpen( *dev, ic4::throwError );

    auto property_map = g.devicePropertyMap();

    for( auto&& e : lst )
    {
        auto f = e.find( '=' );
        if( f == std::string::npos ) continue;

        auto prop_name = e.substr( 0, f );
        auto prop_value = e.substr( f + 1 );

        print( "Trying to set property '{}' to '{}'\n", prop_name, prop_value );

        auto prop = property_map.get( prop_name.c_str() );
        switch( prop.getType() )
        {
        case ic4::PropType::Boolean:
        {
            bool value_to_set = false;
            if( prop_value == "true" ) {
                value_to_set = true;
            }
            else if( prop_value == "false" ) {
                value_to_set = false;
            } else {
                print( "Failed to parse value for property '{}'. Value: '{}'\n", prop_name, prop_value );
                break;
            }
            ic4::Error err;
            if( !prop.asBoolean().setValue( value_to_set, err ) )
            {
                print( "Failed to set value '{}' on property '{}' , due to err:{}\n", value_to_set, prop_name, err.getMessage() );
            }
            break;
        }
        case ic4::PropType::String:
        {
            ic4::Error err;
            if( !prop.asString().setValue( prop_value, err ) )
            {
                print( "Failed to set value '{}' on property '{}' , due to err:{}\n", prop_value, prop_name, err.getMessage() );
            }
            break;
        }
        case ic4::PropType::Command:
        {
            ic4::Error err;
            if( !prop.asCommand().execute( err ) )
            {
                print( "Failed execute on Command property '{}' , due to err:{}\n", prop_name, err.getMessage() );
            }
            break;
        }
        case ic4::PropType::Integer:
        {
            int64_t value_to_set = 0;
            if( !helper::from_chars_helper( prop_value, value_to_set ) ) {
                print( "Failed to parse value for property '{}'. Value: '{}'\n", prop_name, prop_value );
                break;
            }
            ic4::Error err;
            if( !prop.asInteger().setValue( value_to_set, err ) )
            {
                print( "Failed to set value '{}' on property '{}' , due to err:{}\n", value_to_set, prop_name, err.getMessage() );
            }
            break;
        }
        case ic4::PropType::Float:
        {
            double value_to_set = 0;
            if( !helper::from_chars_helper( prop_value, value_to_set ) ) {
                print( "Failed to parse value for property '{}'. Value: '{}'\n", prop_name, prop_value );
                break;
            }
            ic4::Error err;
            if( !prop.asFloat().setValue( value_to_set, err ) )
            {
                print( "Failed to set value '{}' on property '{}' , due to err:{}\n", value_to_set, prop_name, err.getMessage() );
            }
            break;
        }
        case ic4::PropType::Enumeration:
        {
            auto enum_prop = prop.asEnumeration();
            if( enum_prop.findEntry( prop_value ).is_valid() )
            {
                ic4::Error err;
                if( !enum_prop.selectEntry( prop_value, err ) )
                {
                    print( "Failed to select entry '{}' on property '{}' , due to err:{}\n", prop_value, prop_name, err.getMessage() );
                }
            }
            else
            {
                int64_t value_to_set = 0;
                if( !helper::from_chars_helper( prop_value, value_to_set ) ) {
                    print( "Failed to parse value for property '{}'. Value: '{}'\n", prop_name, prop_value );
                    break;
                }

                ic4::Error err;
                if( !enum_prop.setValue( value_to_set, err ) )
                {
                    print( "Failed to set value '{}' on property '{}' , due to err:{}\n", value_to_set, prop_name, err.getMessage() );
                }
            }
            break;
        }
        case ic4::PropType::Category:
        case ic4::PropType::Register:
        case ic4::PropType::Port:
        case ic4::PropType::EnumEntry:
            print( "Cannot set a value for a {} property. Name: '{}'\n", ic4_helper::toString( prop.getType() ), prop_name );
            break;
        case ic4::PropType::Invalid:
            print( "Failed to find property. Name: '{}'\n", prop_name );
            break;
        default:
            print( "Invalid property type. Value: {}\n", static_cast<int>(prop.getType()) );
            break;
        };
    }
}


int main( int argc, char** argv )
{
    CLI::App app{ "Simple ic4 camera control utility", "ic4-ctrl"};
    app.set_help_flag();
    app.set_help_all_flag( "-h,--help", "Expand all help" );

    std::string gentl_path = helper::get_env_var( "GENICAM_GENTL64_PATH" );
    app.add_option( "--gentl-path", gentl_path, "GenTL path environment variable to set." )->default_val( gentl_path );

    std::string arg_device_id;
    auto list_cmd = app.add_subcommand( "list",
        "List available devices.\n"
    );

    auto device_cmd = app.add_subcommand( "device",
        "When no additional parameters are give, all devices are listed otherwise information for the device-id specified is printed.\n"
        "\te.g. `ic4-ctrl interface` lists all device\n"
        "\tOne device can be specified by adding its name or index at the end of the parameter list.\n"
        "\te.g. `ic4-ctrl device \"<id>\"` this lists information about the device identified by <id>."
    );
    device_cmd->allow_extras();
    auto interface_cmd = app.add_subcommand( "interface", 
        "List interfaces.\n"
        "\te.g. `ic4-ctrl interface` lists all interfaces\n"
        "\tOne interface can be specified by adding its name or index at the end of the parameter list.\n"
        "\te.g. `ic4-ctrl interface \"<id>\"` this lists information about the interface identified by <id>."
    );
    interface_cmd->allow_extras();

    auto list_props_cmd = app.add_subcommand( "list-prop", 
        "List properties of device specified by 'ic4-ctrl-cpp list-prop -d <device-id>'. You can specify properties to list by adding their names." );
    list_props_cmd->allow_extras();
    list_props_cmd->add_option( "-d,--device", arg_device_id,
        "Device to open. If '0' is specified (and no device with this id is present), the first device is opened." )->required();

    auto set_props_cmd = app.add_subcommand( "set-prop", "Set a list of properties 'ic4-ctrl-cpp set-prop -d <device-id> ExposureAuto=false Exposure=0.3 '." );
    set_props_cmd->allow_extras();
    set_props_cmd->add_option( "-d,--device", arg_device_id,
        "Device to open. If '0' is specified (and no device with this id is present), the first device is opened." )->required();

    auto save_props_cmd = app.add_subcommand( "save-prop", "Save properties for the specified device 'ic4-ctrl-cpp save-prop -d <device-id>' -f <filename>." );
    std::string arg_filename;
    save_props_cmd->add_option( "-d,--device", arg_device_id,
        "Device to open. If '0' is specified (and no device with this id is present), the first device is opened." )->required();
    save_props_cmd->add_option( "-f,--filename", arg_filename, "Filename to save into." )->required();

    auto image_cmd = app.add_subcommand( "image", 
        "Save one or more images from the specified device 'ic4-ctrl-cpp image -d <device-id> -f <filename> --count 3 --timeout 2000 --type bmp'."
    );
    int count = 1;
    int timeout = 1000;
    std::string image_type = "bmp";
    image_cmd->add_option( "-d,--device", arg_device_id,
        "Device to open. If '0' is specified (and no device with this id is present), the first device is opened." )->required();
    image_cmd->add_option( "-f,--filename", arg_filename, "Filename. Use '{}' to specify where a counter should be placed (e.g. 'test-{}.bmp'." )->required();
    image_cmd->add_option( "--count", count, "Count of frames to capture." )->default_val( count );
    image_cmd->add_option( "--timeout", timeout, "Timeout in milliseconds." )->default_val( timeout );
    image_cmd->add_option( "--type", image_type, "Image file type to save. [bmp,png,jpeg,tiff]" )->default_val( image_type );

    auto live_cmd = app.add_subcommand( "live", "Display live stream. 'ic4-ctrl-cpp live -d <device-id>'." );
    live_cmd->add_option( "-d,--device", arg_device_id, 
        "Device to open. If '0' is specified (and no device with this id is present), the first device is opened." )->required();

    try
    {
        app.parse( argc, argv );
    }
    catch( const CLI::ParseError& e )
    {
        return app.exit( e );
    }

    if( !gentl_path.empty() ) {
        helper::set_env_var( "GENICAM_GENTL64_PATH", gentl_path );
    }

    ic4::InitLibrary();

    try
    {
        if( list_cmd->parsed() )
        {
            list_interfaces();
            print( "\n" );
            list_devices();
        }
        else if( device_cmd->parsed() )
        {
            auto list = list_cmd->remaining();
            if( list.empty() ) {
                list_devices();
            } else {
                print_device( list.front() );
            }
        }
        else if( interface_cmd->parsed() )
        {
            auto list = interface_cmd->remaining();
            if( list.empty() ) {
                list_interfaces();
            } else {
                print_interface( list.front() );
            }
        }
        else if( list_props_cmd->parsed() ) {
            print_properties( arg_device_id, list_props_cmd->remaining() );
        }
        else if( save_props_cmd->parsed() ) {
            save_properties( arg_device_id, arg_filename );
        }
        else if( image_cmd->parsed() ) {
            save_image( arg_device_id, arg_filename, count, timeout, image_type );
        }
        else if( live_cmd->parsed() ) {
            show_live( arg_device_id );
        }
        else if( set_props_cmd->parsed() ) {
            set_props( arg_device_id, set_props_cmd->remaining() );
        }
        else
        {
            print("No arguments given\n\n");
            print("{}\n", app.get_formatter()->make_help(&app, app.get_name(), CLI::AppFormatMode::All));
        }
    }
    catch( const std::exception& ex )
    {
        fmt::print( stderr, "Error: {}\n", ex.what() );
    }

	ic4::ExitLibrary();
	return 0;
}

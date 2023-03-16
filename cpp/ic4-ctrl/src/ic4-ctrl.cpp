
#include <ic4/ic4.h>

#include <CLI/CLI.hpp>
#include <fmt/core.h>

#include <condition_variable>
#include <mutex>
#include <string>

#include "ic4_enum_to_string.h"
#include "ic4-ctrl-helper.h"

template<class ... Targs>
void print( fmt::format_string<Targs...> fmt, Targs&& ... args )
{
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


static auto list_devices() -> void
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.getAvailableVideoCaptureDevices();

    fmt::print( "    {:24} {:8} {}\n", "ModelName", "Serial", "InterfaceName" );
    int index = 0;
    for( auto&& e : list ) {
        fmt::print( "{:>3} {:24} {:8} {}\n", index, e.getModelName(), e.getSerial(), e.getInterface().getTransportLayerName() );
    }
    if( list.empty() ) {
        fmt::print( "    No devices found\n" );
    }
}

static auto list_interfaces() -> void
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.getAvailableInterfaces( ic4::throwError );

    fmt::print( "Interface list:" );

    for( auto&& e : list ) {
        print( "{}\n", e.getName() );
        print( "\tTransportLayerName: {}\n", e.getTransportLayerName() );
        print( "\tTransportVersion: {}\n", e.getTransportVersion() );
        print( "\tTransportLayerType: {}\n", ic4_helper::toString( e.getTransportLayerType() ) );
    }
    if( list.empty() ) {
        fmt::print( "No Interfaces found\n" );
    }
}


static void print_device( std::string id )
{
    auto dev = find_device( id );
    if( !dev ) {
        throw std::runtime_error( fmt::format( "Failed to find device for id '{}'\n", id ) );
    }

    fmt::print( "ModelName: '{}'\n", dev->getModelName() );
    fmt::print( "Serial: '{}'\n", dev->getSerial() );
    fmt::print( "UniqueName: '{}'\n", dev->getUniqueName() );
    fmt::print( "getDeviceVersion: '{}'\n", dev->getDeviceVersion() );
    fmt::print( "InterfaceName: '{}'\n", dev->getInterface().getTransportLayerName() );
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

static void    print_property( const ic4::Property& property )
{
    using namespace ic4_helper;

    auto prop_type = property.getType();
    print( "{:24} - Type: {}, DisplayName: {}\n", property.getName(), toString( prop_type ), property.getDisplayName() );
    print( "\tDescription: {}\n", property.getDescription() );
    print( "\tTooltip: {}\n", property.getTooltip() );
    print( "\tVisibility: {}, Available: {}, Locked: {}, ReadOnly: {}\n", toString( property.getVisibility() ), property.isAvailable(), property.isLocked(), property.isReadOnly() );

    if( property.isSelector( ic4::throwError ) )
    {
        print( "\tSelected properties:\n" );
        for( auto&& selected : property.getSelectedProperties( ic4::throwError ) )
        {
            print( "\t\t{}\n", selected.getName() );
        }
    }

    switch( prop_type )
    {
    case ic4::PropType::Integer:
    {
        ic4::PropInteger prop = property.asInteger();
        auto inc_mode = prop.getIncrementMode();

        print( "\tRepresentation: '{}', Unit: '{}', IncrementMode: '{}'\n", toString( prop.getRepresentation() ), prop.getUnit(), toString( inc_mode ) );

        if( prop.isAvailable() )
        {
            if( !prop.isReadOnly() ) {
                print( "\tMin: {}, Max: {}\n",
                    fetch_PropertyMethod_value<int64_t>( prop, &ic4::PropInteger::getMinimum ),
                    fetch_PropertyMethod_value<int64_t>( prop, &ic4::PropInteger::getMaximum )
                );
            }
            if( inc_mode == ic4::PropIncrementMode::Increment ) {
                print( "\tInc: {}\n",
                    fetch_PropertyMethod_value<int64_t>( prop, &ic4::PropInteger::getIncrement )
                );
            }
            else if( inc_mode == ic4::PropIncrementMode::ValueSet )
            {
                std::vector<int64_t> vvset;
                if( !prop.getValidValueSet( vvset, ic4::ignoreError ) ) {
                    print( "\tFailed to fetch ValidValueSet\n" );
                }
                else
                {
                    print( "\tValidValueSet:" );
                    for( auto&& val : vvset ) {
                        print( "\t\t{}\n", val );
                    }
                    print( "\n" );
                }
            }
            print( "\tValue: {}\n",
                fetch_PropertyMethod_value<int64_t>( prop, &ic4::PropInteger::getValue )
            );
        }
        break;
    }
    case ic4::PropType::Float:
    {
        ic4::PropFloat prop = property.asFloat();
        auto inc_mode = prop.getIncrementMode();

        print( "\tRepresentation: '{}', Unit: '{}', IncrementMode: '{}', DisplayNotation: {}, DisplayPrecision: {}\n",
            toString( prop.getRepresentation() ), prop.getUnit(), toString( inc_mode ), toString( prop.getDisplayNotation() ), prop.getDisplayPrecision() );

        if( prop.isAvailable() )
        {
            if( !prop.isReadOnly() ) {
                print( "\tMin: {}, Max: {}\n",
                    fetch_PropertyMethod_value<double>( prop, &ic4::PropFloat::getMinimum ),
                    fetch_PropertyMethod_value<double>( prop, &ic4::PropFloat::getMaximum )
                );
            }

            if( inc_mode == ic4::PropIncrementMode::Increment ) {
                print( "\tInc: {}\n",
                    fetch_PropertyMethod_value<double>( prop, &ic4::PropFloat::getIncrement )
                );
            }
            else if( inc_mode == ic4::PropIncrementMode::ValueSet )
            {
                //std::vector<double> vvset;
                //if( !prop.getValidValueSet( vvset, ic4::ignoreError ) ) {
                //    print( "\tFailed to fetch ValidValueSet\n" );
                //}
                //else
                //{
                //    print( "\tValidValueSet:" );
                //    for( auto&& val : vvset ) {
                //        print( "\t\t{}\n", val );
                //    }
                //    print( "\n" );
                //}
            }

            print( "\tValue: {}\n",
                fetch_PropertyMethod_value<double>( prop, &ic4::PropFloat::getValue )
            );
        }
        break;
    }
    case ic4::PropType::Enumeration:
    {
        auto prop = property.asEnumeration();
        print( "\tEnumEntries:\n" );
        for( auto&& entry : prop.getEntries( ic4::ignoreError ) )
        {
            auto prop_enum_entry = entry.asEnumEntry();

            print( "\t\t{}, DisplayName: {}\n", prop_enum_entry.getName(), prop_enum_entry.getDisplayName() );
            print( "\t\t\tDescription: {}\n", prop_enum_entry.getDescription() );
            print( "\t\t\tTooltip: {}\n", prop_enum_entry.getTooltip() );
            print( "\t\t\tVisibility: {}, Available: {}, Locked: {}, ReadOnly: {}\n",
                toString( prop_enum_entry.getVisibility() ), prop_enum_entry.isAvailable(), prop_enum_entry.isLocked(), prop_enum_entry.isReadOnly() );

            if( prop_enum_entry.isAvailable() ) {
                print( "\t\t\tValue: {}\n", fetch_PropertyMethod_value<int64_t>( prop_enum_entry, &ic4::PropEnumEntry::getValue ) );
            }
            print( "\n" );
        }

        if( prop.isAvailable() )
        {
            ic4::Error err;
            auto selected_entry = prop.getSelectedEntry( err );
            if( err ) {
                print( "\tValue: {}, SelectedEntry.Name: '{}'\n", "err", "err" );
            }
            else
            {
                print( "\tValue: {}, SelectedEntry.Name: '{}'\n",
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
            print( "\tValue: {}\n",
                fetch_PropertyMethod_value<bool>( prop, &ic4::PropBoolean::getValue )
            );
        }
        break;
    }
    case ic4::PropType::String:
    {
        auto prop = property.asString();

        if( prop.isAvailable() ) {
            print( "\tValue: '{}', MaxLength: {}\n",
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
        print( "\tFeatures:\n" );
        for( auto&& feature : prop.getFeatures( ic4::ignoreError ) )
        {
            print( "\t\t{}\n", feature.getName() );
        }
        break;
    }
    case ic4::PropType::Register:
    {
        ic4::PropRegister prop = property.asRegister();

        print( "\tSize: {}", 
            fetch_PropertyMethod_value<uint64_t>( prop, &ic4::PropRegister::getSize )
        );
        if( prop.isAvailable() ) {
            std::vector<uint8_t> vec;
            if( prop.getValue( vec, ic4::ignoreError ) ) {
                print( "\tValue: 'err'" );
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
                print( "\tValue: [{}], Value-Size: {}\n", str, vec.size() );
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
            print_property( property );
        }
    }
    else
    {
        for( auto&& entry : lst )
        {
            auto property = map.get( entry.c_str() );
            if( property.is_valid() ) {
                print_property( property );
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
    auto list_cmd = app.add_subcommand( "list", "List available devices." );
    list_cmd->add_option( "-d,--device", arg_device_id,
        "Device to open. If '0' is specified (and no device with this id is present), the first device is opened." );

    auto list_gentl_cmd = app.add_subcommand( "list-interfaces", "List available interfaces" );

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
        if( list_cmd->parsed() ) {
            if( arg_device_id.empty() )
                list_devices();
            else
                print_device( arg_device_id );
        }
        else if( list_gentl_cmd->parsed() ) {
            list_interfaces();
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
            fmt::print("No arguments given\n\n");
            fmt::print("{}\n", app.get_formatter()->make_help(&app, app.get_name(), CLI::AppFormatMode::All));
        }
    }
    catch( const std::exception& ex )
    {
        fmt::print( stderr, "Error: {}\n", ex.what() );
    }

	ic4::ExitLibrary();
	return 0;
}

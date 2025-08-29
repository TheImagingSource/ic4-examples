
#include <ic4/ic4.h>

#ifdef _WIN32

#include <ic4-gui/ic4gui.h>

#endif

#include <CLI/CLI.hpp>
#include <fmt/core.h>
#include <fmt/ranges.h>

#include <cstdint>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <stdexcept>

#include "ic4-ctrl-helper.h"
#include "ic4_enum_to_string.h"
#include "print_property.h"

static auto find_device( std::string id ) -> std::unique_ptr<ic4::DeviceInfo>
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.enumDevices();
    if( list.size() == 0 ) {
        throw std::runtime_error( "No devices are available" );
    }

    for( auto&& dev : list )
    {
        if( dev.serial() == id ) {
            return std::make_unique<ic4::DeviceInfo>( dev );
        }
    }
    for( auto&& dev : list )
    {
        if( dev.uniqueName() == id ) {
            return std::make_unique<ic4::DeviceInfo>( dev );
        }
    }
    for( auto&& dev : list )
    {
        if( dev.modelName() == id ) {
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

static auto find_interface( std::string id ) -> std::unique_ptr<ic4::Interface>
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.enumInterfaces();
    if( list.size() == 0 ) {
        throw std::runtime_error( "No devices are available" );
    }

    for( auto&& dev : list )
    {
        if( dev.interfaceDisplayName() == id ) {
            return std::make_unique<ic4::Interface>( dev );
        }
    }
    for( auto&& dev : list )
    {
        if( dev.transportLayerName() == id ) {
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

static auto list_all_by_connection() -> void
{
	ic4::DeviceEnum devEnum;
	auto list = devEnum.enumInterfaces();

	if (list.empty()) {
		print(1, "No GenTL providers found\n");
	}
	else
	{
		std::set<std::string> transport_layer_list;

		for (auto&& e : list) {
			transport_layer_list.insert(e.transportLayerName());
		}
		for (auto&& transportLayerName : transport_layer_list)
		{
			print(0, "TransportLayer: {}\n", transportLayerName);
			for (size_t i = 0; i < list.size(); ++i)
			{
				auto& itf = list.at(i);
				if (transportLayerName == itf.transportLayerName())
				{
					//print(1, "{:64} Index:{:2}\n\n", itf.interfaceDisplayName(), i);
					print(1, "{:2} {}\n", i, itf.interfaceDisplayName());

					auto dev_list = itf.enumDevices();
					if (dev_list.empty()) {
						print(3, "No devices\n");
					}
					else
					{
						for (auto&& device : dev_list) {
							print(3, "{:24} {:8}\n", device.modelName(), device.serial());
						}
					}
					print("\n");
				}
			}
			print("\n");
		}
	}
}

static auto list_devices(bool serials_only) -> void
{
	ic4::DeviceEnum devEnum;
	auto list = devEnum.enumDevices();

	if (serials_only) {
		for (auto&& e : list) {
			print("{} ", e.serial());
		}
	}
	else
	{
		print("Device list:\n");
		if (list.empty()) {
			print(1, "No devices found\n");
			return;
		}

		print(1, "Index   {:24} {:8} {}\n", "ModelName", "Serial", "InterfaceName");
		int index = 0;
		for (auto&& e : list) {
			print(1, "{:^5}   {:24} {:8} {}\n", index, e.modelName(), e.serial(), e.getInterface().transportLayerName());
			index += 1;
		}
	}
}

static auto read_IPAddressList(ic4::PropertyMap& map, ic4::PropInteger& subnet_ip, bool add_mask) -> std::vector<std::string>
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

auto print_interface_short(int offset, size_t id, ic4::Interface& itf) -> void
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
		auto lst = read_IPAddressList(map, ipaddr, false);
		add_info += fmt::format(" IPv4={::}", lst);
	}

	print(offset, "{:^5} {:64}{}\n", id, itf.interfaceDisplayName(), add_info);
}

static auto list_interfaces() -> void
{
    ic4::DeviceEnum devEnum;
    auto list = devEnum.enumInterfaces();

	if (list.empty()) {
		print(1, "No Interfaces found\n");
	}
	else
	{
		std::set<std::string> transport_layer_list;

		for (auto&& e : list) {
			transport_layer_list.insert(e.transportLayerName());
		}
		for (auto&& transportLayerName : transport_layer_list)
		{
			print(0, "TransportLayer: {}\n", transportLayerName);
			for (size_t i = 0; i < list.size(); ++i)
			{
				auto& itf = list.at(i);
				if (transportLayerName == itf.transportLayerName())
				{
					print_interface_short(1, i, itf);
				}
			}
			print("\n");
		}
	}
}

static void print_device( std::string id, bool device_cmd_serials)
{
    auto dev = find_device( id );
    if( !dev ) {
        throw std::runtime_error( fmt::format( "Failed to find device for id '{}'\n", id ) );
    }
	if (device_cmd_serials)
	{
		print("{} ", dev->serial());
	}
	else
	{
		print("ModelName:     '{}'\n", dev->modelName());
		print("Serial:        '{}'\n", dev->serial());

		auto user_id = dev->userID(ic4::Error::Ignore());
		if (!user_id.empty())
		{
			print("UserID:        '{}'\n", user_id);
		}
		else
		{
			print("UserID:        '<empty>'\n");
		}

		print("UniqueName:    '{}'\n", dev->uniqueName());
		print("DeviceVersion: '{}'\n", dev->version());
		print("InterfaceName: '{}'\n", dev->getInterface().transportLayerName());
	}
}

static void print_interface( std::string id )
{
    auto dev = find_interface( id );
    if( !dev ) {
        throw std::runtime_error( fmt::format( "Failed to find device for id '{}'\n", id ) );
    }

    print( "DisplayName:           '{}'\n", dev->interfaceDisplayName() );
    print( "TransportLayerName:    '{}'\n", dev->transportLayerName() );
    print( "TransportLayerType:    '{}'\n", ic4_helper::toString( dev->transportLayerType() ) );
    print( "TransportLayerVersion: '{}'\n", dev->transportLayerVersion() );

	auto map = dev->interfacePropertyMap();

	auto mtu = map.findInteger("MaximumTransmissionUnit", ic4::Error::Ignore());
	if (mtu.is_valid()) {
		print(1, "MaximumTransmissionUnit: '{}'\n", helper::get_value_as_string(mtu));
	}

	auto ipaddr = map.findInteger("GevInterfaceSubnetIPAddress", ic4::Error::Ignore());
	if (ipaddr.is_valid())
	{
		print(1, "GevInterfaceSubnet-info: {::}", read_IPAddressList(map, ipaddr, true));
	}

}

static auto split_prop_entry( const std::string& prop_string ) -> std::pair<std::string,std::string>
{
    auto f = prop_string.find( '=' );
    if( f == std::string::npos ) {
        return { prop_string, std::string{} };
    }

    return { prop_string.substr( 0, f ), prop_string.substr( f + 1 ) };
}

static void set_property_from_assign_entry( ic4::PropertyMap& property_map, const std::string& prop_name, const std::string& prop_value )
{
    print( "Setting property '{}' to '{}'\n", prop_name, prop_value );

    ic4::Error err;
    if (!property_map.setValue(prop_name, prop_value, err))
    {
        print("Failed to set value '{}' on property '{}'. Message: {}\n", prop_value, prop_name, err.message());
    }
}

static void print_or_set_PropertyMap_entries( ic4::PropertyMap& map, const std::vector<std::string>& lst )
{
    if( lst.empty() )
    {
        for( auto&& property : map.all() )
        {
            helper::print_property( 0, property );
        }
    }
    else
    {
        for( auto&& entry : lst )
        {
            auto parse_entry = split_prop_entry( entry );
            if( !parse_entry.second.empty() )
            {
                set_property_from_assign_entry( map, parse_entry.first, parse_entry.second );
            }
            else
            {
                auto property = map.find( entry );
                if( property.is_valid() ) {
                    helper::print_property( 0, property );
                } else {
                    print( "Failed to find property for name: '{}'\n", entry );
                }
            }
        }
    }
}


struct selected_prop_map
{
	ic4::Grabber g;
	ic4::PropertyMap map;
};

static auto select_prop_map(std::string id, bool force_interface, bool device_driver_props) -> selected_prop_map
{
	selected_prop_map rval;
	if (force_interface) {
		auto dev = find_interface(id);
		if (!dev) {
			throw std::runtime_error(fmt::format("Failed to find interface for id '{}'", id));
		}
		rval.map = dev->interfacePropertyMap();
	}
	else
	{
		auto dev = find_device(id);
		if (!dev) {
			throw std::runtime_error(fmt::format("Failed to find device for id '{}'", id));
		}

		rval.g.deviceOpen(*dev);

		if (device_driver_props) {
			rval.map = rval.g.driverPropertyMap();
		}
		else {
			rval.map = rval.g.devicePropertyMap();
		}
	}
	return rval;
}

static void exec_prop_cmd(ic4::PropertyMap& map, const std::vector<std::string>& lst)
{
	print_or_set_PropertyMap_entries(map, lst);
}

static void save_properties(ic4::PropertyMap& map, std::string filename)
{
	map.serialize(filename);
}

static void load_properties(ic4::PropertyMap& map, std::string filename)
{
	ic4::Error err;
	map.deSerialize(filename, err);
	if (err) {
		print("Failed to load file '{}' due to error: {}", filename, err.message());
	}
}

static void save_image( std::string id, std::string filename, int count, int timeout_in_ms, std::string image_type )
{
    auto dev = find_device( id );
    if( !dev ) {
        print( "Failed to find device for id '{}'", id );
        return;
    }
    ic4::Grabber g;
    g.deviceOpen( *dev );

    auto snap_sink = ic4::SnapSink::create();
    g.streamSetup( snap_sink, ic4::StreamSetupOption::AcquisitionStart );

    ic4::Error err;
    auto images = snap_sink->snapSequence( count, timeout_in_ms, err );
    if( err ) {
        if( err.code() == ic4::ErrorCode::Timeout ) {
            print( "Timeout elapsed." );
            // #TODO maybe dissect what to do here.
            return;
        }
        throw err;
    }

    g.acquisitionStop();

    int idx = 0;
    for( auto && image : images )
    {
        std::string actual_filename = filename;
        if( filename.find_first_of( '{' ) != std::string::npos
            && filename.find_first_of( '}' ) != std::string::npos )
        {
            actual_filename = fmt::vformat( filename, fmt::make_format_args( idx ) );
            idx++;
        }
        if( image_type == "bmp" ) {
            ic4::imageBufferSaveAsBitmap( *image, actual_filename, {} );
        }
        else if( image_type == "png" ) {
            ic4::imageBufferSaveAsPng( *image, actual_filename, {} );
        }
        else if( image_type == "tiff" ) {
            ic4::imageBufferSaveAsTiff( *image, actual_filename, {} );
        }
        else if( image_type == "jpeg" ) {
            ic4::imageBufferSaveAsJpeg( *image, actual_filename, {} );
        }
    }
}

#ifdef _WIN32

static void show_live( std::string id )
{
    auto dev = find_device( id );
    if( !dev ) {
        print( "Failed to find device for id '{}'", id );
        return;
    }
    ic4::Grabber g;
    g.deviceOpen( *dev );

    auto display = ic4::Display::create( ic4::DisplayType::Default, IC4_WINDOW_HANDLE_NULL );
    g.streamSetup( display, ic4::StreamSetupOption::AcquisitionStart );

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
    g.acquisitionStop();
}

static void show_prop_page(ic4::PropertyMap& map, bool show_guru)
{
	ic4gui::PropertyDialogOptions opt = {};
	if (show_guru) {
		opt.initial_visibility = ic4::PropVisibility::Guru;
	}

	ic4gui::showPropertyDialog(0, map, opt);
}

#endif // _WIN32

static void show_version()
{
    auto str = ic4::getVersionInfo();

    if (str.empty())
    {
        print("Unable to retrieve version information.");
        return;
    }

    print("{}", str);
}

static void show_system_info()
{
    auto env_var = helper::get_env_var( "GENICAM_GENTL64_PATH" );
    print( 0, "Environment:\n" );
    print( 1, "GENICAM_GENTL64_PATH: {}\n", env_var );
}

int main( int argc, char** argv )
{
    CLI::App app{ "Simple ic4 camera control utility", "ic4-ctrl"};
    //app.set_help_flag("-h,--help");
    app.set_help_all_flag( "--help-all", "Expand all help" );

    std::string gentl_path = helper::get_env_var( "GENICAM_GENTL64_PATH" );
    app.add_option( "--gentl-path", gentl_path, "GenTL path environment variable to set." )->default_val( gentl_path );

    std::string arg_device_id;
    bool force_interface = false;
	bool props_device_driver = false;
	bool device_cmd_serials = false;
	std::string arg_filename;

	auto help = app.add_subcommand("help", "Print this help text and quit.")->silent();

    auto list_cmd = app.add_subcommand( "list",
        "List available devices and interfaces by connection."
    );

    auto device_cmd = app.add_subcommand( "device",
        "List devices or show information for a single device.\n"
        "\tTo list all devices use: `ic4-ctrl device`\n"
        "\tTo show only a specific device: `ic4-ctrl device \"<id>\"\n"
    );
    auto device_cmd_device_id = device_cmd->add_option( "device-id", arg_device_id,
        "If specified only information for this device is printed, otherwise all device are listed. You can specify an index e.g. '0'." );

	device_cmd->add_flag("--serials", device_cmd_serials, "Return only the serial number of the devices");

    auto interface_cmd = app.add_subcommand( "interface",
        "List devices or a show information for a single interface.\n"
        "\tTo list all interfaces: `ic4-ctrl interface`\n"
        "\tTo show only a specific interface: `ic4-ctrl interface \"<id>\"\n"
    );
    interface_cmd->add_option( "interface-id", arg_device_id,
        "If specified only information for this interface is printed, otherwise all interfaces are listed. You can specify an index e.g. '0'." );

    auto props_cmd = app.add_subcommand( "prop",
        "List or set property values of the specified device or interface.\n"
        "\tTo list all device properties 'ic4-ctrl prop <device-id>'.\n"
        "\tTo list specific device properties 'ic4-ctrl prop <device-id> ExposureAuto ExposureTime'.\n"
        "\tTo set specific device properties 'ic4-ctrl prop <device-id> ExposureAuto=Off ExposureTime=0.5'."
	);
	props_cmd->allow_extras();
	props_cmd->add_flag("--interface", force_interface, "If set the <device-id> is interpreted as an interface-id.");
	props_cmd->add_flag("--device-driver", props_device_driver, "If set the device instance driver properties are used.");
	props_cmd->add_option("device-id", arg_device_id,
		"Specifies the device to open. You can specify an index e.g. '0'.")->required();


	auto save_props_cmd = app.add_subcommand( "save-prop", 
        "Save properties for the specified device 'ic4-ctrl save-prop -f <filename> <device-id>'." );
    save_props_cmd->add_option( "-f,--filename", arg_filename, "Filename to save into." )->required();
    save_props_cmd->add_option( "device-id", arg_device_id, "Specifies the device to open. You can specify an index e.g. '0'." )->required();
	save_props_cmd->add_flag("--device-driver", props_device_driver, "If set the device instance driver properties are used.");

	auto load_props_cmd = app.add_subcommand("load-prop",
		"Load properties for the specified device 'ic4-ctrl load-prop -f <filename> <device-id>'.");
	load_props_cmd->add_option("-f,--filename", arg_filename, "Filename to save into.")->required();
	load_props_cmd->add_option("device-id", arg_device_id, "Specifies the device to open. You can specify an index e.g. '0'.")->required();
	load_props_cmd->add_flag("--interface", force_interface, "If set the <device-id> is interpreted as an interface-id.");
	load_props_cmd->add_flag("--device-driver", props_device_driver, "If set the device instance driver properties are used.");

    auto image_cmd = app.add_subcommand( "image", 
        "Save one or more images from the specified device 'ic4-ctrl image -f <filename> --count 3 --timeout 2000 --type bmp <device-id>'."
    );
    int count = 1;
    int timeout = 1000;
    std::string image_type = "bmp";
    image_cmd->add_option( "-f,--filename", arg_filename, "Filename. Use '{}' to specify where a counter should be placed (e.g. 'test-{}.bmp'." )->required();
    image_cmd->add_option( "--count", count, "Count of frames to capture." )->default_val( count );
    image_cmd->add_option( "--timeout", timeout, "Timeout in milliseconds." )->default_val( timeout );
    image_cmd->add_option( "--type", image_type, "Image file type to save. [bmp,png,jpeg,tiff]" )->default_val( image_type );
	image_cmd->add_flag("--interface", force_interface, "If set the <device-id> is interpreted as an interface-id.");
	image_cmd->add_option("device-id", arg_device_id,
        "Specifies the device to open. You can specify an index e.g. '0'." )->required();

#ifdef _WIN32

    auto live_cmd = app.add_subcommand( "live", "Display a live stream. 'ic4-ctrl live <device-id>'." );
    live_cmd->add_option( "device-id", arg_device_id,
        "Specifies the device to open. You can specify an index e.g. '0'." )->required();

	bool show_default_guru = false;
    auto show_prop_page_cmd = app.add_subcommand( "show-prop", "Display the property page for the device or interface id. 'ic4-ctrl show-prop <id>'." );
    show_prop_page_cmd->add_option( "device-id", arg_device_id,
        "Specifies the device to open. You can specify an index e.g. '0'." )->required();
    show_prop_page_cmd->add_flag( "--interface", force_interface,
        "If set the <device-id> is interpreted as an interface-id." );
	show_prop_page_cmd->add_flag("--device-driver", props_device_driver, "If set the device instance driver properties are used.");
	show_prop_page_cmd->add_flag("-g,--guru", show_default_guru,
		"Start the dialog with Visibility set to ic4::PropVisibility::Guru.");

#endif // _WIN32

    auto system_cmd = app.add_subcommand( "system",
        "List some information for about the system."
    );

    auto version_cmd = app.add_subcommand( "version",
        "List version information about IC4."
    );

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

    ic4::InitLibraryConfig config =
    {
        ic4::ErrorHandlerBehavior::Throw,
        ic4::LogLevel::Off
    };
    ic4::initLibrary(config);


    try
    {

		if (help->count() != 0)
		{
			app.exit(CLI::CallForAllHelp());
			return 0;
		}

        if( list_cmd->parsed() )
        {
			list_all_by_connection();
        }
        else if( device_cmd->parsed() )
        {
            if( arg_device_id.empty() ) {
                list_devices(device_cmd_serials);
            } else {
				print_device(arg_device_id, device_cmd_serials);
            }
        }
        else if( interface_cmd->parsed() )
        {
            if( arg_device_id.empty() ) {
                list_interfaces();
            } else {
				print_interface(arg_device_id);
            }
        }
        else if( props_cmd->parsed() )
        {
			auto prop_map = select_prop_map(arg_device_id, force_interface, props_device_driver);
            exec_prop_cmd(prop_map.map, props_cmd->remaining() );
        }
        else if( save_props_cmd->parsed() )
        {
			auto prop_map = select_prop_map(arg_device_id, force_interface, props_device_driver);
			save_properties(prop_map.map, arg_filename);
        }
		else if (load_props_cmd->parsed())
		{
			auto prop_map = select_prop_map(arg_device_id, force_interface, props_device_driver);
			load_properties(prop_map.map, arg_filename);
		}
        else if( image_cmd->parsed() ) {
            save_image( arg_device_id, arg_filename, count, timeout, image_type );
        }
#ifdef _WIN32
        else if( live_cmd->parsed() )
        {
            show_live(arg_device_id);
        }
        else if( show_prop_page_cmd->parsed() )
        {
			auto prop_map = select_prop_map(arg_device_id, force_interface, props_device_driver);
			show_prop_page(prop_map.map, show_default_guru);
        }
#endif // _WIN32
        else if( system_cmd->parsed() )
        {
            show_system_info();
        }
        else if( version_cmd->parsed() )
        {
            show_version();
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

	ic4::exitLibrary();

	return 0;
}

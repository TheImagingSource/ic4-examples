
cmake_minimum_required(VERSION 3.10)

project("device-enumeration")

find_package( ic4 REQUIRED )

add_executable( device-enumeration 
	"src/device-enumeration.cpp"
)

target_include_directories( device-enumeration	PRIVATE		"../../common" )
target_link_libraries( device-enumeration		PRIVATE		ic4::core )
set_target_properties( device-enumeration		PROPERTIES	CXX_STANDARD 14 )


ic4_copy_runtime_to_target(device-enumeration)
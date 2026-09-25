
cmake_minimum_required(VERSION 3.10)

project("ip-configuration")

find_package( ic4 REQUIRED )

add_executable( ip-configuration "src/ip-configuration.cpp" )

target_include_directories( ip-configuration	PRIVATE		"../../common" )
target_link_libraries( ip-configuration			PRIVATE		ic4::core )
set_target_properties( ip-configuration			PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(ip-configuration)

cmake_minimum_required(VERSION 3.10)

project("usersets")

find_package( ic4 REQUIRED )

add_executable( usersets "src/usersets.cpp" )

target_include_directories( usersets	PRIVATE		"../../common" )
target_link_libraries( usersets			PRIVATE		ic4::core )
set_target_properties( usersets			PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(usersets)

cmake_minimum_required(VERSION 3.10)

project("lut-control")

find_package( ic4 REQUIRED )

add_executable( lut-control "src/lut-control.cpp" )

target_include_directories( lut-control	PRIVATE		"../../common" )
target_link_libraries( lut-control		    PRIVATE		ic4::core )
set_target_properties( lut-control		    PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(lut-control)
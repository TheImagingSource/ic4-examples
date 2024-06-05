
cmake_minimum_required(VERSION 3.8)

project("event-line1-edge")

find_package( ic4 REQUIRED )

add_executable( event-line1-edge "src/event-line1-edge.cpp" )

target_include_directories( event-line1-edge	PRIVATE		"../../common" )
target_link_libraries( event-line1-edge		    PRIVATE		ic4::core )
set_target_properties( event-line1-edge		    PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(event-line1-edge)
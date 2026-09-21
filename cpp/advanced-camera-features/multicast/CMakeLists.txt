
cmake_minimum_required(VERSION 3.10)

project("multicast")

find_package( ic4 REQUIRED )

add_executable( multicast-master "src/multicast.cpp" )

target_include_directories( multicast-master	PRIVATE		"../../common" )
target_link_libraries( multicast-master			PRIVATE		ic4::core )
target_compile_definitions(multicast-master		PRIVATE		IC4_MULTICAST_MASTER)
set_target_properties( multicast-master			PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(multicast-master)


add_executable( multicast-monitor "src/multicast.cpp" )

target_include_directories( multicast-monitor	PRIVATE		"../../common" )
target_link_libraries( multicast-monitor		PRIVATE		ic4::core )
target_compile_definitions(multicast-monitor	PRIVATE		IC4_MULTICAST_MONITOR)
set_target_properties( multicast-monitor		PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(multicast-monitor)
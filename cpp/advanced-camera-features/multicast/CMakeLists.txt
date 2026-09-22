
cmake_minimum_required(VERSION 3.10)

project("multicast")

find_package( ic4 REQUIRED )

add_executable( multicast-master "src/multicast-master.cpp" )

target_include_directories( multicast-master	PRIVATE		"../../common" )
target_link_libraries( multicast-master			PRIVATE		ic4::core )
set_target_properties( multicast-master			PROPERTIES	CXX_STANDARD 17 )

ic4_copy_runtime_to_target(multicast-master)


add_executable( multicast-monitor "src/multicast-monitor.cpp" )

target_include_directories( multicast-monitor	PRIVATE		"../../common" )
target_link_libraries( multicast-monitor		PRIVATE		ic4::core )
set_target_properties( multicast-monitor		PROPERTIES	CXX_STANDARD 17 )

ic4_copy_runtime_to_target(multicast-monitor)
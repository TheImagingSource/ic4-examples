
cmake_minimum_required(VERSION 3.10)

project("event-exposure-end")

find_package( ic4 REQUIRED )

add_executable( event-exposure-end
    "src/event-exposure-end.cpp"
)

target_include_directories( event-exposure-end	PRIVATE		"../../common" )
target_link_libraries( event-exposure-end		PRIVATE		ic4::core )
set_target_properties( event-exposure-end		PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(event-exposure-end)
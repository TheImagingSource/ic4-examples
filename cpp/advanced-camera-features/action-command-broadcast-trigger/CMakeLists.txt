
cmake_minimum_required(VERSION 3.8)

project("action-command-broadcast-trigger")

if( NOT TARGET ic4::core )
    find_package( ic4 REQUIRED )
endif()

add_executable( action-command-broadcast-trigger 
    "src/action-command-broadcast-trigger.cpp"
)

target_include_directories( action-command-broadcast-trigger	PRIVATE		"../../common" )
target_link_libraries( action-command-broadcast-trigger			PRIVATE		ic4::core )
set_target_properties( action-command-broadcast-trigger			PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(action-command-broadcast-trigger)
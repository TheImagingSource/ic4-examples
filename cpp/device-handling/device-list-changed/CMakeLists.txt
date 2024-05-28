
cmake_minimum_required(VERSION 3.8)

project("device-list-changed")

if( NOT TARGET ic4::core )
	find_package( ic4 REQUIRED )
endif()

add_executable( device-list-changed 
	"src/device-list-changed.cpp"
)

target_include_directories( device-list-changed	PRIVATE		"../../common" )
target_link_libraries( device-list-changed		PRIVATE		ic4::core )
set_target_properties( device-list-changed		PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(device-list-changed)
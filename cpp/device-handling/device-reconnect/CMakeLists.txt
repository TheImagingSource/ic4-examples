
cmake_minimum_required(VERSION 3.10)

project("device-reconnect")

find_package( ic4 REQUIRED )

add_executable( device-reconnect 
	"src/device-reconnect.cpp"
)

target_include_directories( device-reconnect	PRIVATE		"../../common" )
target_link_libraries( device-reconnect			PRIVATE		ic4::core )
set_target_properties( device-reconnect			PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(device-reconnect)
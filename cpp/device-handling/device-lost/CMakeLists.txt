
cmake_minimum_required(VERSION 3.10)

project("device-lost")

find_package( ic4 REQUIRED )

add_executable( device-lost 
	"src/device-lost.cpp"
)

target_include_directories( device-lost PRIVATE		"../../common" )
target_link_libraries( device-lost 		PRIVATE		ic4::core )
set_target_properties( device-lost 		PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(device-lost)
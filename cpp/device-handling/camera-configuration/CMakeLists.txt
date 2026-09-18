
cmake_minimum_required(VERSION 3.10)

project("camera-configuration")

find_package( ic4 REQUIRED )

add_executable( camera-configuration 
	"src/camera-configuration.cpp"
)

target_include_directories( camera-configuration	PRIVATE		"../../common" )
target_link_libraries( camera-configuration			PRIVATE		ic4::core )
set_target_properties( camera-configuration			PROPERTIES	CXX_STANDARD 14 )


ic4_copy_runtime_to_target(camera-configuration)
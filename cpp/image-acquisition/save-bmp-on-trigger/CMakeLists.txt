
cmake_minimum_required(VERSION 3.10)

project("save-bmp-on-trigger")

find_package( ic4 REQUIRED )

add_executable( save-bmp-on-trigger 
	"src/save-bmp-on-trigger.cpp"
)

target_include_directories( save-bmp-on-trigger PRIVATE		"../../common" )
target_link_libraries( save-bmp-on-trigger 		PRIVATE		ic4::core )
set_target_properties( save-bmp-on-trigger 		PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(save-bmp-on-trigger)

cmake_minimum_required(VERSION 3.8)

project("save-bmp-on-trigger")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

add_executable( save-bmp-on-trigger 
	"src/save-bmp-on-trigger.cpp"
)

include_directories( save-bmp-on-trigger
	"../../common"
)

target_link_libraries( save-bmp-on-trigger
PRIVATE
	ic4::core
)

set_property(TARGET save-bmp-on-trigger PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(save-bmp-on-trigger)

cmake_minimum_required(VERSION 3.8)

project("device-list-changed")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

add_executable( device-list-changed 
	"src/device-list-changed.cpp"
)

target_link_libraries( device-list-changed
PRIVATE
	ic4::core
)

set_property(TARGET device-list-changed PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(device-list-changed)
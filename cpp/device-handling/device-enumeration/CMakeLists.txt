
cmake_minimum_required(VERSION 3.8)

project("device-enumeration")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

add_executable( device-enumeration 
	"src/device-enumeration.cpp"
)

target_link_libraries( device-enumeration
PRIVATE
	ic4::core
)

set_property(TARGET device-enumeration PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(device-enumeration)
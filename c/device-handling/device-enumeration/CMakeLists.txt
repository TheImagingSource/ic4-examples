
cmake_minimum_required(VERSION 3.8)

project("device-enumeration-c")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

add_executable( device-enumeration-c 
	"src/device-enumeration.c"
)

target_link_libraries( device-enumeration-c
PRIVATE
	ic4::core
)

ic4_copy_runtime_to_target(device-enumeration-c)
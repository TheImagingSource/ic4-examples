
cmake_minimum_required(VERSION 3.10)

project("device-enumeration-c")

find_package( ic4 REQUIRED )

add_executable( device-enumeration-c 
	"src/device-enumeration.c"
)

target_link_libraries( device-enumeration-c
PRIVATE
	ic4::core
)

ic4_copy_runtime_to_target(device-enumeration-c)

cmake_minimum_required(VERSION 3.8)

project("device-list-changed-c")

find_package( ic4 REQUIRED )

add_executable( device-list-changed-c 
	"src/device-list-changed.c"
)

target_link_libraries( device-list-changed-c
PRIVATE
	ic4::core
)

ic4_copy_runtime_to_target(device-list-changed-c)
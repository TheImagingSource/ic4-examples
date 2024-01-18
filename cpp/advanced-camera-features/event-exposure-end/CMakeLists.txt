
cmake_minimum_required(VERSION 3.8)

project("event-exposure-end")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

add_executable( event-exposure-end
	"src/event-exposure-end.cpp"
)

include_directories( event-exposure-end
	"../../common"
)

target_link_libraries( event-exposure-end
PRIVATE
	ic4::core
)

set_property(TARGET event-exposure-end PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(event-exposure-end)
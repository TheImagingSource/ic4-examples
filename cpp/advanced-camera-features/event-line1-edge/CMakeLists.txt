
cmake_minimum_required(VERSION 3.8)

project("event-line1-edge")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

add_executable( event-line1-edge 
	"src/event-line1-edge.cpp"
)

include_directories( event-line1-edge
	"../../common"
)

target_link_libraries( event-line1-edge
PRIVATE
	ic4::core
)

set_property(TARGET event-line1-edge PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(event-line1-edge)

cmake_minimum_required(VERSION 3.8)

project("connect-chunkdata")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

add_executable( connect-chunkdata 
	"src/connect-chunkdata.cpp"
)

include_directories( connect-chunkdata
	"../../common"
)

target_link_libraries( connect-chunkdata
PRIVATE
	ic4::core
)

set_property(TARGET connect-chunkdata PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(connect-chunkdata)
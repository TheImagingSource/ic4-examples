
cmake_minimum_required(VERSION 3.8)

project("record-mp4-h264")

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

add_executable( record-mp4-h264 
	"src/record-mp4-h264.cpp"
)

include_directories( record-mp4-h264
	"../../common"
)

target_link_libraries( record-mp4-h264
PRIVATE
	ic4::core
)

set_property(TARGET record-mp4-h264 PROPERTY CXX_STANDARD 14)

ic4_copy_runtime_to_target(record-mp4-h264)
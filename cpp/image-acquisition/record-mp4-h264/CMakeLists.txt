
cmake_minimum_required(VERSION 3.8)

project("record-mp4-h264")

find_package( ic4 REQUIRED )

add_executable( record-mp4-h264 
	"src/record-mp4-h264.cpp"
)

target_include_directories( record-mp4-h264 PRIVATE		"../../common" )
target_link_libraries( record-mp4-h264 		PRIVATE		ic4::core )
set_target_properties( record-mp4-h264 		PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(record-mp4-h264)
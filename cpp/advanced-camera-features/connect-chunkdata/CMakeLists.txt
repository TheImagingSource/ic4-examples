
cmake_minimum_required(VERSION 3.8)

project("connect-chunkdata")

find_package( ic4 REQUIRED )

add_executable( connect-chunkdata 
    "src/connect-chunkdata.cpp"
)

target_include_directories( connect-chunkdata	PRIVATE		"../../common" )
target_link_libraries( connect-chunkdata		PRIVATE		ic4::core )
set_target_properties( connect-chunkdata		PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(connect-chunkdata)
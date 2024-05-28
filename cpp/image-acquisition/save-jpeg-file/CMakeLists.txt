
cmake_minimum_required(VERSION 3.8)

project("save-image-file")

if( NOT TARGET ic4::core )
	find_package( ic4 REQUIRED )
endif()

add_executable( save-jpeg-file 
	"src/save-jpeg-file.cpp"
)

target_include_directories( save-jpeg-file PRIVATE		"../../common" )
target_link_libraries( save-jpeg-file 		PRIVATE		ic4::core )
set_target_properties( save-jpeg-file 		PROPERTIES	CXX_STANDARD 14 )

ic4_copy_runtime_to_target(save-jpeg-file)
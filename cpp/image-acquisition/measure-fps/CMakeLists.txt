
cmake_minimum_required(VERSION 3.10)

project("measure-fps")

find_package( ic4 REQUIRED )

add_executable( measure-fps
	"src/measure-fps.cpp"
)

target_include_directories( measure-fps PRIVATE		"../../common" )
target_link_libraries( measure-fps 		PRIVATE		ic4::core )
set_target_properties( measure-fps 		PROPERTIES	CXX_STANDARD 14 )

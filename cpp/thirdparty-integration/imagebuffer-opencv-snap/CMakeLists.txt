
cmake_minimum_required(VERSION 3.23)

project("imagebuffer-opencv-snap")

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


find_package( OpenCV REQUIRED )
find_package( ic4 REQUIRED )

add_executable( imagebuffer-opencv-snap 
	"src/imagebuffer-opencv-snap.cpp"
)

target_include_directories( imagebuffer-opencv-snap
	PRIVATE "../../common"
)
target_link_libraries( imagebuffer-opencv-snap
	PRIVATE ic4::core
	PRIVATE opencv_core
	PRIVATE opencv_imgproc
	PRIVATE opencv_imgcodecs
	PRIVATE opencv_highgui
)
set_target_properties( imagebuffer-opencv-snap
	PROPERTIES CXX_STANDARD 14
)

ic4_copy_runtime_to_target(imagebuffer-opencv-snap)
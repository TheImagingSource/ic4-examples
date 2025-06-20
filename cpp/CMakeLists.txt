
cmake_minimum_required(VERSION 3.21)

project( ic4-examples-cpp )

find_package( ic4 CONFIG REQUIRED )

add_subdirectory( advanced-camera-features/action-command-broadcast-trigger )
add_subdirectory( advanced-camera-features/connect-chunkdata )
add_subdirectory( advanced-camera-features/event-exposure-end )
add_subdirectory( advanced-camera-features/event-line1-edge )

add_subdirectory( device-handling/device-enumeration )
add_subdirectory( device-handling/device-list-changed )
add_subdirectory( device-handling/device-lost )

add_subdirectory( image-acquisition/record-mp4-h264 )
add_subdirectory( image-acquisition/save-bmp-on-trigger )
add_subdirectory( image-acquisition/save-jpeg-file )
add_subdirectory( image-acquisition/measure-fps )

add_subdirectory( qt6 )

find_package( OpenCV CONFIG )
if( OpenCV_FOUND )
	add_subdirectory( thirdparty-integration/imagebuffer-opencv-snap )
endif()


cmake_minimum_required(VERSION 3.16)

project(cpp-qt6-examples)

find_package( ic4 CONFIG REQUIRED )

include( ./common/setup_qt.cmake )

add_subdirectory( common/qt6-dialogs )
add_subdirectory( demoapp )
add_subdirectory( device-manager )

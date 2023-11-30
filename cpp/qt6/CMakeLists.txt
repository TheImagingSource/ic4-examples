
cmake_minimum_required(VERSION 3.16)

project(cpp-qt6-examples) 

find_package(ic4 REQUIRED)

# This path probably will not be valid on your system.
# If cmake complains about not able to find Qt6, point it into your Qt6 installation directory.
list(APPEND CMAKE_PREFIX_PATH "c:/source/qt/qt6/6.5.2/msvc2019_64/")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_subdirectory( common/qt6-dialogs )
add_subdirectory( demoapp )

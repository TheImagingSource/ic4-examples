
cmake_minimum_required(VERSION 3.16)

project(cpp-qt6-examples)

find_package(ic4 REQUIRED)

# in theory all ic4 modules are qt5 compatible
# it is encouraged to switch to qt6 wherever possible
# set this to "Qt5" to compile against qt5 instead of qt6
set (IC4_USED_QT_VERSION "Qt6")

if (IC4_USED_QT_VERSION STREQUAL "Qt6")

    # This path probably will not be valid on your system.
    # If cmake complains about not able to find Qt6, point it into your Qt6 installation directory.
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")

    find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

else ()

    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

endif ()

add_subdirectory( common/qt6-dialogs )
add_subdirectory( demoapp )
add_subdirectory( device-manager )

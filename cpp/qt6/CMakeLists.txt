
cmake_minimum_required(VERSION 3.21)

project(cpp-qt6-examples)

find_package( ic4 CONFIG REQUIRED )

set(QT_REQUIRED_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

include( ./common/setup_qt.cmake )

if( QT_VERSION )

    add_subdirectory( common/qt6-dialogs )
    add_subdirectory( demoapp )
    add_subdirectory( device-manager )
    add_subdirectory( high-speed-capture )

else()

    message(STATUS "Qt examples need an installed Qt development version. Skipping qt-dialogs based projects." )

endif()
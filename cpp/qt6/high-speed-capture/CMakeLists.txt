
cmake_minimum_required(VERSION 3.21)

project(high-speed-capture)

find_package( ic4 CONFIG REQUIRED )

set(QT_REQUIRED_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

include( ../common/setup_qt.cmake )

if( NOT Qt6_FOUND )
    if(PROJECT_IS_TOP_LEVEL)
        message( FATAL_ERROR "HighSpeedCapture depends on Qt6")
    else()
        message( WARNING "The HighSpeedCapture example needs at least Qt6")
    endif()
    
    return()
endif()

find_package(Qt6 REQUIRED COMPONENTS Concurrent)

if( NOT TARGET qt6-dialogs )
    add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()

add_executable(high-speed-capture
    "HighSpeedCaptureDialog.h"
    "HighSpeedCaptureDialog.cpp"
    "main.cpp"
)

target_link_libraries(high-speed-capture PRIVATE Qt6::Core Qt6::Widgets Qt6::Concurrent)
target_link_libraries(high-speed-capture PRIVATE ic4::core qt6-dialogs )

set_target_properties(high-speed-capture PROPERTIES CXX_STANDARD 17 )

if (WIN32)

    set_target_properties(high-speed-capture PROPERTIES WIN32_EXECUTABLE ON )

    ic4_copy_runtime_to_target(high-speed-capture)

    add_windeployqt_custom_command(high-speed-capture) # Adds a POST_BUILD call to windeployqt.exe 

endif ()

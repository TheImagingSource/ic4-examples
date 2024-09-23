
cmake_minimum_required(VERSION 3.21)

project(high-speed-capture)

find_package( ic4 CONFIG REQUIRED )

include( ../common/setup_qt.cmake )


if( ${QT_VERSION} VERSION_LESS 6.1.0)
    if(PROJECT_IS_TOP_LEVEL)
        message( FATAL_ERROR "HighSpeedCapture depends on Qt6")
    else()
        message( WARNING "The HighSpeedCapture example needs at least Qt6")
    endif()
    
    return()
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Concurrent)

if( NOT TARGET qt6-dialogs )
    add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()

add_executable(high-speed-capture
    "HighSpeedCaptureDialog.h"
    "HighSpeedCaptureDialog.cpp"
    "main.cpp"
)

target_link_libraries(high-speed-capture PRIVATE Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Concurrent)
target_link_libraries(high-speed-capture PRIVATE ic4::core qt6-dialogs )

set_target_properties(high-speed-capture PROPERTIES CXX_STANDARD 17 )

if (WIN32)

    set_target_properties(high-speed-capture PROPERTIES WIN32_EXECUTABLE ON )

    ic4_copy_runtime_to_target(high-speed-capture)

    add_custom_command(TARGET high-speed-capture POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
            --verbose 0
            --no-compiler-runtime
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            $<TARGET_FILE:high-speed-capture>
        COMMENT "Deploying Qt..."
    )

endif ()

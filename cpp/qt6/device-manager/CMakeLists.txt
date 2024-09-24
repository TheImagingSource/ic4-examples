
cmake_minimum_required(VERSION 3.16)

project(device-manager)

find_package( ic4 CONFIG REQUIRED )

include( ../common/setup_qt.cmake )

if( NOT TARGET qt6-dialogs )
    add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()

add_executable(ic4-device-manager
    "main.cpp"
    "device-manager.rc"
)

target_link_libraries(ic4-device-manager PRIVATE Qt::Core Qt::Widgets  )
target_link_libraries(ic4-device-manager PRIVATE ic4::core qt6-dialogs )

set_target_properties(ic4-device-manager PROPERTIES CXX_STANDARD 17 )

if (WIN32)

    set_target_properties(ic4-device-manager PROPERTIES WIN32_EXECUTABLE ON )

    ic4_copy_runtime_to_target(ic4-device-manager)

    add_custom_command(TARGET ic4-device-manager POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
            --verbose 0
            --no-compiler-runtime
            --no-translations
            --no-system-d3d-compiler
            --no-opengl-sw
            $<TARGET_FILE:ic4-device-manager>
        COMMENT "Deploying Qt..."
    )

endif ()


cmake_minimum_required(VERSION 3.16)

project(demoapp)

set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )

find_package( ic4 CONFIG REQUIRED )

include( ../common/setup_qt.cmake )

if( NOT TARGET qt6-dialogs )
    add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()


qt_add_resources(DEMOAPP_RESOURCES demoapp.qrc)

add_executable(ic4-demoapp
    ${DEMOAPP_RESOURCES}
    "main.cpp"
    "mainwindow.cpp"
    "events.h"
    "demoapp.rc"
)

set_target_properties(ic4-demoapp PROPERTIES CXX_STANDARD 17 )
target_link_libraries(ic4-demoapp PRIVATE ic4::core qt6-dialogs )
target_link_libraries(ic4-demoapp PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core )


# Qt6 has separated the OpenGL widget from the Widgets component so we have to add it here
if (QT_VERSION_MAJOR STREQUAL "6")

    find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)

    target_link_libraries(ic4-demoapp PRIVATE Qt6::OpenGLWidgets)

endif ()

if (WIN32)
    target_link_libraries(ic4-demoapp PRIVATE shell32 )

    set_target_properties(ic4-demoapp PROPERTIES WIN32_EXECUTABLE ON )

    ic4_copy_runtime_to_target(ic4-demoapp)

    add_custom_command(TARGET ic4-demoapp POST_BUILD
        COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
        --verbose 0
        --no-compiler-runtime
        --no-translations
        --no-system-d3d-compiler
        --no-opengl-sw
        $<TARGET_FILE:ic4-demoapp>
        COMMENT "Deploying Qt..."
    )
endif()

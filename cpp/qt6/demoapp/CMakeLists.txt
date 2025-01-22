
cmake_minimum_required(VERSION 3.21)

project(demoapp)

find_package( ic4 CONFIG REQUIRED )

set(QT_REQUIRED_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

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

target_include_directories(ic4-demoapp PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

if (WIN32)
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

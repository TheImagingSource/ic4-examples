cmake_minimum_required(VERSION 3.16)
project(demoapp) 

set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )

find_package(ic4)

# This path probably will not be valid on your system.
# If cmake complains about not able to find Qt6, point it into your Qt6 installation directory.
list(APPEND CMAKE_PREFIX_PATH "P:/lib/Qt6/6.6.0/msvc2019_64/lib/cmake")

find_package(Qt6 COMPONENTS Widgets Core)
if (Qt6Widgets_FOUND)
    if (Qt6Widgets_VERSION VERSION_LESS 6.0)
        message(FATAL_ERROR "Minimum supported Qt6 version is 6.0" ${Qt6_DIR})
    endif()
else()
    message(SEND_ERROR "The Qt6Widgets library could not be found!")
endif( Qt6Widgets_FOUND)

if( NOT TARGET qt6-dialogs )
add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()

qt_standard_project_setup()
qt_add_resources(DEMOAPP_RESOURCES demoapp.qrc)

qt_add_executable(demoapp
    ${DEMOAPP_RESOURCES}
    main.cpp
    mainwindow.cpp
    events.h
)

target_link_libraries(demoapp
    PRIVATE
        Qt6::Widgets
        Qt6::Core
        ic4::core
        qt6-dialogs
        shell32
)

set_target_properties(demoapp PROPERTIES
    WIN32_EXECUTABLE ON
    CXX_STANDARD 17
)

ic4_copy_runtime_to_target(demoapp)

add_custom_command(TARGET demoapp POST_BUILD
    COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
        --verbose 0
        --no-compiler-runtime
        --no-translations
        --no-system-d3d-compiler
        --no-opengl-sw
        $<TARGET_FILE:demoapp>
    COMMENT "Deploying Qt..."
)

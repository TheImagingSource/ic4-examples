cmake_minimum_required(VERSION 3.16)
project(demoapp) 

set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

# This path probably will not be valid on your system.
# If cmake complains about not able to find Qt6, point it into your Qt6 installation directory.
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

if( NOT TARGET qt6-dialogs )
add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()

qt_standard_project_setup()

qt_add_executable(device-manager
    main.cpp
    device-manager.rc
)

target_link_libraries(device-manager
    PRIVATE
        Qt6::Widgets
        Qt6::Core
        ic4::core
        qt6-dialogs
)

set_target_properties(device-manager PROPERTIES
    WIN32_EXECUTABLE ON
    CXX_STANDARD 17
)

ic4_copy_runtime_to_target(device-manager)

add_custom_command(TARGET device-manager POST_BUILD
    COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
        --verbose 0
        --no-compiler-runtime
        --no-translations
        --no-system-d3d-compiler
        --no-opengl-sw
        $<TARGET_FILE:demoapp>
    COMMENT "Deploying Qt..."
)

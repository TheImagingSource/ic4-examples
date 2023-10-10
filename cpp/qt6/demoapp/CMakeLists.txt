cmake_minimum_required(VERSION 3.16)
project(demoapp) 

find_package(ic4)

# This path probably will not be valid on your system.
# If cmake complains about not able to find Qt6, point it at your installation directory.
find_package(Qt6 COMPONENTS Widgets Core PATHS "P:/lib/Qt6/6.6.0/msvc2019_64/lib/cmake")
if (Qt6Widgets_FOUND)
    if (Qt6Widgets_VERSION VERSION_LESS 6.0)
        message(FATAL_ERROR "Minimum supported Qt6 version is 6.0" ${Qt6_DIR})
    endif()
else()
    message(SEND_ERROR "The Qt6Widgets library could not be found!")
endif( Qt6Widgets_FOUND)

qt_standard_project_setup()
qt_add_resources(DEMOAPP_RESOURCES demoapp.qrc)

qt_add_executable(demoapp
    ${DEMOAPP_RESOURCES}
    main.cpp
    mainwindow.cpp
    ic4dialogs/deviceselection/deviceselection.cpp
    uitils.cpp utils.h
    events.h
    ic4dialogs/propertydlg/propertydlg.cpp
    ic4dialogs/propertydlg/propertydlg.h
)

add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/qtdlg)

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

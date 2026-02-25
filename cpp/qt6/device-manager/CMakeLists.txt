
cmake_minimum_required(VERSION 3.16)

project(device-manager)

find_package( ic4 CONFIG REQUIRED )

set(QT_REQUIRED_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

include( ../common/setup_qt.cmake )

if( NOT TARGET qt6-dialogs )
    add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()

add_executable(ic4-device-manager
    "main.cpp"
    "device-manager.rc"
)

target_link_libraries(ic4-device-manager PRIVATE ic4::core qt6-dialogs )
target_link_libraries(ic4-device-manager PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core )

set_target_properties(ic4-device-manager PROPERTIES CXX_STANDARD 17 )

if(WIN32)

    set_target_properties(ic4-device-manager PROPERTIES WIN32_EXECUTABLE ON )

    ic4_copy_runtime_to_target(ic4-device-manager)

    add_windeployqt_custom_command(ic4-device-manager) # Adds a POST_BUILD call to windeployqt.exe 

elseif(APPLE)

    set_target_properties(ic4-device-manager PROPERTIES MACOSX_BUNDLE ON )

    set(myApp_ICON "${CMAKE_CURRENT_SOURCE_DIR}/images/tis.ico")
    target_sources(ic4-device-manager PRIVATE ${myApp_ICON})
    set_source_files_properties(${myApp_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    set_target_properties(ic4-device-manager PROPERTIES 
        MACOSX_BUNDLE_ICON_FILE "tis.ico"
        MACOSX_BUNDLE_BUNDLE_NAME "com.tis.ic4-device-manager"
    )

    set_target_properties(ic4-device-manager PROPERTIES 
        MACOSX_BUNDLE_BUNDLE_NAME "ic4-device-manager"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright The Imaging Source Europe GmbH"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.tis.ic4-device-manager"
        MACOSX_BUNDLE_ICON_FILE "tis.ico"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_FULL_VERSION}"
    )

endif()

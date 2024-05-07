cmake_minimum_required(VERSION 3.16)
project(device-manager)

set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )

if( NOT TARGET ic4::core )
    find_package( ic4 REQUIRED )
endif()

if (IC4_USED_QT_VERSION STREQUAL "Qt6")

    # This path probably will not be valid on your system.
    # If cmake complains about not able to find Qt6, point it into your Qt6 installation directory.
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")

    find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

    if (Qt6Core_VERSION VERSION_LESS 6.3.0)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
    else()
    	qt_standard_project_setup()
    endif()

else ()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
endif ()

if( NOT TARGET qt6-dialogs )
    add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()


if (IC4_USED_QT_VERSION STREQUAL "Qt6")
    qt_add_executable(ic4-device-manager
        main.cpp
        device-manager.rc
      )

    target_link_libraries(ic4-device-manager
        PRIVATE
        Qt6::Widgets
        Qt6::Core
      )

else ()
    add_executable(ic4-device-manager
        main.cpp
        device-manager.rc
      )

    target_link_libraries(ic4-device-manager
        PRIVATE
        Qt5::Widgets
        Qt5::Core
      )

endif ()

# always needed
target_link_libraries(ic4-device-manager
    PRIVATE
    ic4::core
    qt6-dialogs
  )

if (WIN32)

set_target_properties(ic4-device-manager PROPERTIES
    WIN32_EXECUTABLE ON
    CXX_STANDARD 17
)

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

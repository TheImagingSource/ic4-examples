cmake_minimum_required(VERSION 3.16)
project(demoapp)

set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )

if( NOT TARGET ic4::core )
find_package( ic4 REQUIRED )
endif()

if (IC4_USED_QT_VERSION STREQUAL "Qt6")

    # This path probably will not be valid on your system.
    # If cmake complains about not able to find Qt6, point it into your Qt6 installation directory.
    list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")
    find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

    if (Qt6Core_VERSION VERSION_LESS 6.3.0)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
    else()
        qt_standard_project_setup()
    endif()

    qt_add_resources(DEMOAPP_RESOURCES demoapp.qrc)
else ()

    find_package(Qt5 REQUIRED COMPONENTS Core Widgets ) # OpenGLWidgets is part of Widgets in qt5
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)

    qt5_add_resources(DEMOAPP_RESOURCES demoapp.qrc)
endif ()

if( NOT TARGET qt6-dialogs )
    add_subdirectory(../common/qt6-dialogs ${CMAKE_BINARY_DIR}/demoapp-qt6-dialogs)
endif()

add_executable(ic4-demoapp
    ${DEMOAPP_RESOURCES}
    main.cpp
    mainwindow.cpp
    events.h
    demoapp.rc
)

target_link_libraries(ic4-demoapp
    PRIVATE
        ic4::core
        qt6-dialogs
      )

if (IC4_USED_QT_VERSION STREQUAL "Qt6")

    target_link_libraries(ic4-demoapp
        PRIVATE
        Qt6::Widgets
        Qt6::Core
        Qt6::OpenGLWidgets
    )

else ()

    target_link_libraries(ic4-demoapp
        PRIVATE
        Qt5::Widgets
        Qt5::Core
    )

endif ()

if (WIN32)
  target_link_libraries(ic4-demoapp
    PRIVATE
    shell32
  )


  set_target_properties(ic4-demoapp PROPERTIES
    WIN32_EXECUTABLE ON
    CXX_STANDARD 17
  )

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

cmake_minimum_required(VERSION 3.16)
project(dolpsegmentation) 

set(PROJECT_NAME dolpsegmentation)

# set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )
find_package( ic4 REQUIRED )

# This path probably will not be valid on your system.
# If cmake complains about not able to find Qt6, point it into your Qt6 installation directory.
list(APPEND CMAKE_PREFIX_PATH "c:/source/qt/qt6/6.5.2/msvc2019_64/lib/cmake/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")

set(QT_REQUIRED_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

include( ../../qt6/common/setup_qt.cmake )


find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

if( NOT TARGET qt6-dialogs )
    add_subdirectory(../../qt6/common/qt6-dialogs ${CMAKE_BINARY_DIR}/qt6-dialogs)
endif()

# qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}
    main.cpp
    mainwindow.cpp
    sliderctrl.h
    main.rc
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::Core
        ic4::core
        qt6-dialogs
      )

if (WIN32)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    shell32
  )


  set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    CXX_STANDARD 17
  )

  ic4_copy_runtime_to_target(${PROJECT_NAME})

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${Qt6_DIR}/../../../bin/windeployqt.exe"
    --verbose 0
    --no-compiler-runtime
    --no-translations
    --no-system-d3d-compiler
    --no-opengl-sw
    $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Deploying Qt..."
  )
endif()

cmake_minimum_required(VERSION 3.21)

project(qt6-dialogs)

find_package( ic4 CONFIG REQUIRED )

set(QT_REQUIRED_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

include( ../setup_qt.cmake )

qt_add_resources(QT6DIALOGS_RESOURCES qt6dialogs.qrc)

add_library(qt6-dialogs STATIC
    ${QT6DIALOGS_RESOURCES}
    controls/Event.h
    controls/FormGroupBox.h
    controls/IPConfigGroupBox.cpp
    controls/IPConfigGroupBox.h
    controls/PropertyInfoBox.h
    controls/SwitchDriverGroupBox.cpp
    controls/SwitchDriverGroupBox.h
    controls/PropertyControls.h
    controls/PropertyControls.cpp
    controls/props/PropIntControl.h
    controls/props/PropIntSpinBox.h
    controls/props/PropEnumerationControl.h
    controls/props/PropControlBase.h
    controls/props/PropStringControl.h
    controls/props/PropCommandControl.h
    controls/props/PropBooleanControl.h
    controls/props/PropFloatControl.h
    controls/props/PropCategoryControl.h
    controls/PropertyTreeWidget.h

    DeviceSelectionDialog.h
    DeviceSelectionDialog.cpp
    PropertyDialog.h
    PropertyDialog.cpp
)

target_link_libraries(qt6-dialogs PUBLIC Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core )
target_link_libraries(qt6-dialogs PUBLIC ic4::core )

target_include_directories(qt6-dialogs INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )

set_target_properties(qt6-dialogs PROPERTIES CXX_STANDARD 17)

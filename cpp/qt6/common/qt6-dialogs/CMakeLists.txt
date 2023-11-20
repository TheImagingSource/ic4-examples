cmake_minimum_required(VERSION 3.16)
project(qt6-dialogs)

set( CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" )

find_package(ic4)

# This path probably will not be valid on your system.
# If cmake complains about not able to find Qt6, point it into your Qt6 installation directory.
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()
qt_add_resources(QT6DIALOGS_RESOURCES qt6dialogs.qrc)

qt_add_library(qt6-dialogs STATIC
	${QT6DIALOGS_RESOURCES}
	controls/Event.h
	controls/FormGroupBox.h
	controls/IPConfigGroupBox.cpp
	controls/IPConfigGroupBox.h
	controls/PropertyInfoBox.h
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

target_include_directories(qt6-dialogs 
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(qt6-dialogs PUBLIC Qt6::Widgets Qt6::Core ic4::core)

set_property(TARGET qt6-dialogs PROPERTY CXX_STANDARD 17)

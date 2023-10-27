cmake_minimum_required(VERSION 3.16)
project(qt6-dialogs)

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

qt_standard_project_setup()
qt_add_resources(QT6DIALOGS_RESOURCES qt6dialogs.qrc)

qt_add_library(qt6-dialogs STATIC
	${QT6DIALOGS_RESOURCES}
	controls/Event.h
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
	deviceselection.h
	deviceselection.cpp
    controls/IPConfigGroupBox.cpp
	controls/IPConfigGroupBox.h
)

target_include_directories(qt6-dialogs 
	PUBLIC /
)

target_link_libraries(qt6-dialogs PUBLIC Qt6::Widgets Qt6::Core ic4::core)

set_property(TARGET qt6-dialogs PROPERTY CXX_STANDARD 17)

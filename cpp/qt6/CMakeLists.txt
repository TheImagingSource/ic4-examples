
cmake_minimum_required(VERSION 3.16)

project(cpp-qt6-examples)

find_package( ic4 CONFIG REQUIRED )

include( ./common/setup_qt.cmake )

if(QT_FOUND)
	add_subdirectory( common/qt6-dialogs )
	add_subdirectory( demoapp )
	add_subdirectory( device-manager )
	add_subdirectory( high-speed-capture )
else()
	if(WIN32)
		message(WARNING "No Qt5 or Qt6 installation found. To build Qt examples, install Qt. \n"
			"On Windows, add the path to your Qt installation to CMAKE_PREFIX_PATH in cpp/qt6/common/setup_qt.cmake")
	else()
		message(WARNING "No Qt5 or Qt6 installation found. To build Qt examples, install Qt.")
	endif()
endif()

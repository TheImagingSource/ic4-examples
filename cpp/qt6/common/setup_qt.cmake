
# This path probably will not be valid on your system.
# If cmake complains about not able to find Qt, point it into your Qt installation directory.
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.1/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.2/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.1/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.2/msvc2019_64/")	# not yet released
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.2/msvc2019_64/")	# not yet released

# systems like jetpack5 (ubuntu20) only offer qt5
# check both since we want to test in either case
#find_package(Qt6 QUIET COMPONENTS Widgets)
#find_package(Qt5 QUIET COMPONENTS Widgets)

find_package(QT NAMES Qt6 Qt5 REQUIRE COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)


set (IC4_USED_QT_VERSION "Qt${QT_VERSION_MAJOR}") # prefer Qt6

if(Qt6Widgets_FOUND)
    if (Qt6Core_VERSION VERSION_LESS 6.3.0)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
    else()
        qt_standard_project_setup()
    endif()
elseif(Qt5Widgets_FOUND)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
endif()

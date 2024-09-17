
# This path probably will not be valid on your system.
# If cmake complains about not being able to find Qt, point it into your Qt installation directory.
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2019_64/")	# not yet released
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.2/msvc2019_64/")	# not yet released
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.1/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.2/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.1/msvc2019_64/")
list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")

# systems like jetpack5 (ubuntu20) only offer qt5 check both since we want to test in either case
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets)

if(Qt6Widgets_FOUND)
    if (Qt6Core_VERSION VERSION_LESS 6.3.0)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)
    else()
        qt_standard_project_setup() # Added in 6.3.0
    endif()
elseif(Qt5Widgets_FOUND)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
endif()

if( QT_FOUND )
    if(${QT_VERSION} VERSION_LESS 5.15.0) # Version < 5.15. do not have qt_add_resources so replace it with an indirect call
        macro(qt_add_resources)
            qt5_add_resources(${ARGV})
        endmacro()
    endif()
endif()
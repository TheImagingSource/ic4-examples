
if((NOT Qt6_FOUND) AND (NOT Qt5_FOUND) )

    if(WIN32)
        # This path probably will not be valid on your system.
        # If cmake complains about not being able to find Qt, point it into your Qt installation directory.
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.3/msvc2019_64/")	# not yet released
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.2/msvc2019_64/")	# not yet released
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.7.1/msvc2019_64/")
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.3/msvc2019_64/")
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.2/msvc2019_64/")
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.1/msvc2019_64/")
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.6.0/msvc2019_64/")
    endif()

    # systems like jetpack5 (ubuntu20) only offer qt5 check both since we want to test in either case
    find_package(Qt6 QUIET COMPONENTS Core)
    if (Qt6_FOUND)
        find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
        if (Qt6Core_VERSION VERSION_LESS 6.3.0)
            set(CMAKE_AUTOMOC ON)
            set(CMAKE_AUTORCC ON)
            set(CMAKE_AUTOUIC ON)
        else()
            qt_standard_project_setup() # Added in 6.3.0
        endif()

        if( NOT QT_VERSION )
            set( QT_VERSION ${Qt6Core_VERSION} )
        endif()
    else()
        find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)

        if(${Qt5Core_VERSION} VERSION_LESS 5.15.0) # Version < 5.15. do not have qt_add_resources so replace it with an indirect call
            macro(qt_add_resources)
                qt5_add_resources(${ARGV})
            endmacro()
        endif()

        if( NOT QT_VERSION )
            set( QT_VERSION ${Qt5Core_VERSION} )
        endif()
    endif()

    message( STATUS "QT_VERSION=${QT_VERSION} found")

endif()
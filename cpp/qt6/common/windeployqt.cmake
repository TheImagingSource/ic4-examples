
if(WIN32)
    # Macro to add a POST_BUILD custom-command to call windeployqt on the target executable
    function(add_windeployqt_custom_command target_name)
        if(IC4_SKIP_WINDEPLOYQT)
            return()
        endif()
    
        if(NOT IC4_WINDEPLOYQT_EXE)
            set(IC4_WINDEPLOYQT_EXE  "${Qt6_DIR}/../../../bin/windeployqt.exe")
        endif()

        if(NOT EXISTS "${IC4_WINDEPLOYQT_EXE}" )
			message(WARNING "windeployqt not found")
		endif()

        if(IC4_WINDEPLOYQT_PATHS)
            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND "${IC4_WINDEPLOYQT_EXE}"
                --qtpaths "${IC4_WINDEPLOYQT_PATHS}"
                --verbose 0
                --no-compiler-runtime
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                $<TARGET_FILE:${target_name}>
                COMMENT "Deploying Qt..."
            )
        else()

            add_custom_command(TARGET ${target_name} POST_BUILD
                COMMAND "${IC4_WINDEPLOYQT_EXE}"
                --verbose 0
                --no-compiler-runtime
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                $<TARGET_FILE:${target_name}>
                COMMENT "Deploying Qt..."
            )
        endif()
    endfunction()

else()

    function(add_windeployqt_custom_command target_name)
    endfunction()

endif()
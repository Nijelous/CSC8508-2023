function(Create_PC_EntryPoint_Files)  
    message("Entry Point PC")
    ################################################################################
    # Source groups
    ################################################################################


    set(Source_Files
        "main.cpp"
    )

    source_group("Source Files" FILES ${Source_Files})

    set(ALL_FILES
        ${Source_Files}
    )

    ################################################################################
    # Target
    ################################################################################

    add_executable(${PROJECT_NAME}  ${ALL_FILES})

    #use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
    set(ROOT_NAMESPACE EntryPoint)
    #
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_GLOBAL_KEYWORD "Win32Proj"
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )

    ################################################################################
    # Compile definitions
    ################################################################################
    if(MSVC)
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            "UNICODE;"
            "_UNICODE" 
            "WIN32_LEAN_AND_MEAN"
            "_WINSOCKAPI_"   
            "_WINSOCK2API_"
            "_WINSOCK_DEPRECATED_NO_WARNINGS"
        )
    endif()

    target_precompile_headers(${PROJECT_NAME} PRIVATE
        <vector>
        <map>
        <stack>
        <list>   
        <set>   
        <string>
        <thread>
        <atomic>
        <functional>
        <iostream>
        <chrono>
        <sstream>

        "../NCLCoreClasses/Vector2i.h"
        "../NCLCoreClasses/Vector3i.h"
        "../NCLCoreClasses/Vector4i.h"

        "../NCLCoreClasses/Vector2.h"
        "../NCLCoreClasses/Vector3.h"
        "../NCLCoreClasses/Vector4.h"
        "../NCLCoreClasses/Quaternion.h"
        "../NCLCoreClasses/Plane.h"
        "../NCLCoreClasses/Matrix2.h"
        "../NCLCoreClasses/Matrix3.h"
        "../NCLCoreClasses/Matrix4.h"

        "../NCLCoreClasses/GameTimer.h"
    )


    ################################################################################
    # Compile and link options
    ################################################################################
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /Oi;
                /Gy
            >
            /permissive-;
            /std:c++latest;
            /sdl;
            /W3;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            ${DEFAULT_CXX_EXCEPTION_HANDLING};
            /Y-
        )
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
        )
    endif()

    ################################################################################
    # Dependencies
    ################################################################################
    if(MSVC)
        target_link_libraries(${PROJECT_NAME} LINK_PUBLIC  "Winmm.lib")
    endif()

    include_directories("../CSC8503")
    include_directories("../OpenGLRendering/")
    include_directories("../NCLCoreClasses/")
    include_directories("../CSC8503CoreClasses/")
    include_directories("../Recast")
    include_directories("../Detour")
    include_directories("../DebugUtils")
    include_directories("../DetourTileCache")
    include_directories("../FMODCoreAPI/includes")

    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC OpenGLRendering)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Recast)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Detour)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC DebugUtils)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC DetourTileCache)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "../FMODCoreAPI/libs/fmod_vc")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC "../FMODCoreAPI/libs/fmodL_vc")

    file(GLOB DLLS "../FMODCoreAPI/dlls/*.dll")
    foreach(DLL ${DLLS})
            add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${DLL} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endforeach(DLL)
endfunction()
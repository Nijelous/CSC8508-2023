set(PROJECT_NAME EntryPoint)


# PC CMake file
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    message("PCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC")
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
endif()

# PS5 Cmake file

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Prospero")
    message("Ps5555555555555555555555555555555555555555555555555555")

    ################################################################################
    # Source groups
    ################################################################################
    
    file(GLOB_RECURSE Header_Files CONFIGURE_DEPENDS *.h)
    source_group("Header Files" FILES ${Header_Files})

    file(GLOB_RECURSE Source_Files CONFIGURE_DEPENDS *.cpp)
    source_group("Source Files" FILES ${Source_Files})

    file(GLOB_RECURSE ALL_SHADERS CONFIGURE_DEPENDS 
        ${COMPILE_ASSET_ROOT}Shaders/PSSL/*.pssl 
    )
    source_group("Shader Files" FILES ${ALL_SHADERS})


    file(GLOB_RECURSE ALL_TEXTURES CONFIGURE_DEPENDS 
        ${COMPILE_ASSET_ROOT}Textures/*.jpg 
        ${COMPILE_ASSET_ROOT}Textures/*.png 
        ${COMPILE_ASSET_ROOT}Textures/*.dds 
    )
    source_group("Texture Files" FILES ${ALL_TEXTURES})

    set(ALL_FILES
        ${Header_Files}
        ${Source_Files}
        ${ALL_SHADERS}
        ${ALL_TEXTURES}
    )

    ################################################################################
    # Target
    ################################################################################

    add_executable(${PROJECT_NAME} ${ALL_FILES})

    target_precompile_headers(${PROJECT_NAME} PRIVATE
        <memory>
        <unordered_set>
        <vector>
        <map>
        <set>

        <string>
        <fstream>
        <iostream>
        <iosfwd>

        <Matrix.h>
        <Vector.h>
        <Quaternion.h>
        
        <Plane.h>
        <Frustum.h>

        <Camera.h>
        <GameTimer.h>
        <TextureLoader.h>
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
        VS_DEBUGGER_WORKING_DIRECTORY "$(SolutionDir)"
    )

    ################################################################################
    # Compile definitions
    ################################################################################
    if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING};
    )
    endif()

    ################################################################################
    # Dependencies
    ################################################################################
    
    target_include_directories (${PROJECT_NAME} 
    PUBLIC ${CMAKE_SOURCE_DIR}/NCLCoreClasses
    PUBLIC ${CMAKE_SOURCE_DIR}/PS5Core
    PUBLIC ${CMAKE_SOURCE_DIR}/GLTFLoader
	
	PUBLIC ${CMAKE_SOURCE_DIR}/CSC8503CoreClasses
	PUBLIC ${CMAKE_SOURCE_DIR}/CSC8503
    )

    include_directories("../CSC8503")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503)

    # Set the tool and GenerateHeader on PSSL files in ${PSSL_SHADERS}
    set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_TOOL_OVERRIDE WavePsslc)
    set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_SETTINGS "GenerateHeader=false")
    set_property(SOURCE ${ALL_SHADERS} PROPERTY VS_SETTINGS "OutputFileName=${COMPILE_ASSET_ROOT}Shaders/PSSL/%(FileName).ags")
    set_property(SOURCE ${ALL_SHADERS} PROPERTY OUTPUT_NAME "$(Test)%(FileName).ags")


    # Set FileType to Document for all files using the WavePsslc tool
    set_property(TARGET EntryPoint PROPERTY VS_SOURCE_SETTINGS_WavePsslc "FileType=Document")	

    add_custom_target(
        Shaders
    )

    foreach (file ${ALL_TEXTURES})
        get_filename_component(file_name ${file} NAME)
        get_filename_component(file_ext ${file} EXT)
        
        message("Adding custom command to ${file}")
        get_filename_component(file_dir ${file} ABSOLUTE)
        #set(SPIRV_OUTPUT ${file_name}.spv)
        set(TEX_INPUT ${file_dir})
        set(TEX_OUTPUT ${file_dir}.gnf)
        
        message("Reading from ${TEX_INPUT}")
        message("Writing to ${TEX_OUTPUT}")
        
        add_custom_command(
            OUTPUT ${TEX_OUTPUT}
            
            COMMENT "Generating GNF for texture ${file}"
            COMMAND image2gnf.exe -g 1 -i "${file}" -o ${TEX_OUTPUT} -f Bc1UNorm
            DEPENDS ${file}
            VERBATIM
        )
        list(APPEND TEX_GNF_FILES ${TEX_OUTPUT})   
    endforeach()

    add_custom_target(
        Textures
        DEPENDS ON ${TEX_GNF_FILES}
    )
    set(PROJECT_DEPENDENCIES
        NCLCoreClasses
        PS5Core
        GLTFLoader
        CSC8503CoreClasses
    )

    add_dependencies(${PROJECT_NAME}
        ${PROJECT_DEPENDENCIES}
        Shaders
        Textures
    )

    target_link_libraries(${PROJECT_NAME} 
        PRIVATE ${PROJECT_DEPENDENCIES}
        PRIVATE SceAgcDriver_stub_weak
        PRIVATE SceAgc_stub_weak
        PRIVATE SceVideoOut_stub_weak
        PRIVATE SceAgc_debug_nosubmission
        PRIVATE SceAgcCore_debug_nosubmission
        PRIVATE SceAgcGpuAddress_debug_nosubmission
        
        PRIVATE ScePad_stub_weak
        PRIVATE SceUserService_stub_weak
    )

    include_directories(${COMPILE_ASSET_ROOT}) 

endif()
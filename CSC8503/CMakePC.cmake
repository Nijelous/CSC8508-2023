function(Create_PC_CSC8503_Files)

    message("PC CSC8503")

    ################################################################################
    # Source groups
    ################################################################################
    set(Header_Files
        "GameTechRenderer.h"
        "LevelManager.h"
        "NetworkedGame.h"
        "NetworkPlayer.h"
        "StateGameObject.h"
        "DebugNetworkedGame.h"
        "GameSceneManager.h"
        "SinglePlayerStates.h"
        "Scene.h"
        "SceneManager.h"
        "SceneStates.h"
        "MainMenuScene.h"
        "SoundManager.h"
        "MultiplayerStates.h"
        "BaseUI.h"
        "WindowsUI.h"
        "ControllerInterface.h"
        "MiniMap.h"
    )
    source_group("Header Files" FILES ${Header_Files})

    set(Inventory_Buff_System
        "InventoryBuffSystem/InventoryBuffSystem.h"
        "InventoryBuffSystem/PlayerInventory.h"
        "InventoryBuffSystem/PlayerInventory.cpp"
        "InventoryBuffSystem/PlayerBuffs.h"
        "InventoryBuffSystem/PlayerBuffs.cpp"
        "InventoryBuffSystem/PickupGameObject.h"
        "InventoryBuffSystem/PickupGameObject.cpp"    
        "InventoryBuffSystem/FlagGameObject.h"
        "InventoryBuffSystem/FlagGameObject.cpp"
        "InventoryBuffSystem/SoundEmitter.h"
        "InventoryBuffSystem/SoundEmitter.cpp"
        "InventoryBuffSystem/Item.h"
        "InventoryBuffSystem/Item.cpp"
    )
    source_group("Inventory Buff System" FILES ${Inventory_Buff_System})

    set(Suspicion_System
        "SuspicionSystem/GlobalSuspicionMetre.h"
        "SuspicionSystem/GlobalSuspicionMetre.cpp"
        "SuspicionSystem/LocationBasedSuspicion.h"
        "SuspicionSystem/LocationBasedSuspicion.cpp"
        "SuspicionSystem/LocalSuspicionMetre.h"
        "SuspicionSystem/LocalSuspicionMetre.cpp"
        "SuspicionSystem/SuspicionMetre.h"
        "SuspicionSystem/SuspicionMetre.cpp"
        "SuspicionSystem/SuspicionSystem.h"
    )
    source_group("Suspicion System" FILES ${Suspicion_System})

    set(Source_Files
        "GameTechRenderer.cpp"
        "LevelManager.cpp"
        "NetworkedGame.cpp"
        "NetworkPlayer.cpp"
        "StateGameObject.cpp"
        "DebugNetworkedGame.cpp"
        "GameSceneManager.cpp"
        "SinglePlayerStates.cpp"
        "Scene.cpp"
        "SceneManager.cpp"
        "SceneStates.cpp"
        "MainMenuScene.cpp"
        "SoundManager.cpp"
        "MultiplayerStates.cpp"
        "BaseUI.cpp"
        "WindowsUI.cpp"
        "ControllerInterface.cpp"
        "MiniMap.cpp"
    )


    file(GLOB SHADER_FILES ${ASSET_ROOT}/Shaders/VK/*.*)

    source_group("Source Files" FILES ${Source_Files})

    set(ALL_FILES
        ${Header_Files}
        ${Source_Files}
        ${Inventory_Buff_System}
        ${Suspicion_System}
    )

    foreach (file ${SHADER_FILES})
        get_filename_component(file_name ${file} NAME)
	    get_filename_component(file_ext ${file} EXT)
	
	    if(file_ext STREQUAL ".h" OR file_ext STREQUAL ".cpp")
		    continue()
	    endif()
		
	    if( file_ext STREQUAL  ".vert" OR
		    file_ext STREQUAL  ".frag" OR
		    file_ext STREQUAL  ".comp" OR
		    file_ext STREQUAL  ".geom" OR
		    file_ext STREQUAL  ".tesc" OR
		    file_ext STREQUAL  ".tese" OR
		    file_ext STREQUAL  ".rgen" OR
		    file_ext STREQUAL  ".rint" OR
		    file_ext STREQUAL  ".rahit" OR
		    file_ext STREQUAL  ".rchit" OR
		    file_ext STREQUAL  ".rmiss" OR
		    file_ext STREQUAL  ".rcall" OR
		    file_ext STREQUAL  ".task" OR
		    file_ext STREQUAL  ".mesh"
	    )
		    message("Adding custom command to ${file}")
		    get_filename_component(file_dir ${file} ABSOLUTE)
            set(SPIRV_OUTPUT ${file_name}.spv)
		    set(SPIRV_ABS_INPUT ${file_dir})
		    set(SPIRV_ABS_OUTPUT ${file_dir}.spv)
		
		    add_custom_command(
			    OUTPUT ${SPIRV_ABS_OUTPUT}
			
			    COMMENT "Compiling GLSL shader:"
			    COMMAND ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} -V  ${SPIRV_ABS_INPUT} -o ${SPIRV_ABS_OUTPUT}
                DEPENDS ${file}
			    VERBATIM
		    )
            list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUTPUT})   
	    endif()
    endforeach()

    ################################################################################
    # Target
    ################################################################################
    #add_executable(${PROJECT_NAME}  ${ALL_FILES})
    add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

    #use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
    set(ROOT_NAMESPACE CSC8503)

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

    include_directories("../OpenGLRendering/")
    include_directories("../NCLCoreClasses/")
    include_directories("../CSC8503CoreClasses/")
    include_directories("../Recast")
    include_directories("../Detour")
    include_directories("../DebugUtils")
    include_directories("../DetourTileCache")
    include_directories("../FMODCoreAPI/includes")

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
function(Create_PS5_CSC8503_Files)

    message("Ps5 CSC8503")

    ################################################################################
    # Source groups
    ################################################################################
    set(Header_Files
        "GameTechRenderer.h"
        "GameTechAGCRenderer.h"
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
        #"SoundManager.h"
        #"SoundObject.h"
        "ControllerInterface.h"
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
        #"SuspicionSystem/SuspicionSystem.cpp"
    )
    source_group("Suspicion System" FILES ${Suspicion_System})

    set(Source_Files
        "GameTechRenderer.cpp"
         "GameTechAGCRenderer.cpp"
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
        #"SoundManager.cpp"
        #"SoundObject.cpp"
        "GameStart.cpp"
        "ControllerInterface.cpp"
    )

    source_group("Source Files" FILES ${Source_Files})

    set(ALL_FILES
        ${Header_Files}
        ${Source_Files}
        ${Inventory_Buff_System}
        ${Suspicion_System}
    )
    ################################################################################
    # Target
    ################################################################################
    add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

    set(ROOT_NAMESPACE CSC8503)

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
        <string>
        <list>
        <thread>
        <atomic>
        <functional>
        <iostream>
        <set>
	
        "../NCLCoreClasses/Vector.h"
        "../NCLCoreClasses/Quaternion.h"
        "../NCLCoreClasses/Plane.h"
        "../NCLCoreClasses/Matrix.h"
        "../NCLCoreClasses/GameTimer.h" 	
    )

    ################################################################################
    # Dependencies
    ################################################################################
    
    include_directories("../NCLCoreClasses/")
    include_directories("../CSC8503CoreClasses/")
    include_directories("../Recast")
    include_directories("../Detour")
    include_directories("../DebugUtils")
    include_directories("../DetourTileCache")
    include_directories("../PS5Core")

    target_link_libraries(${PROJECT_NAME} PRIVATE ${NCLCoreClasses})
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC CSC8503CoreClasses)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Recast)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC Detour)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC DebugUtils)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC DetourTileCache)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC PS5Core)

endfunction()
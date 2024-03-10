function(Create_CSC8503CoreClasses_Files_PS5)
    message("PS5 CSC8503CoreClasses")

    set(PROJECT_NAME CSC8503CoreClasses)

    ################################################################################
    # Source groups
    ################################################################################
    set(AI_Behaviour_Tree
        "BehaviourAction.h"
        "BehaviourNode.h"
        "BehaviourNodeWithChildren.h"
        "BehaviourSelector.h"
        "BehaviourSelector.cpp"
        "BehaviourSequence.h"
        "BehaviourSequence.cpp"
    )
    source_group("AI\\Behaviour Trees" FILES ${AI_Behaviour_Tree})

    set(AI_Pushdown_Automata
        "PushdownMachine.h"
        "PushdownMachine.cpp"
        "PushdownState.h"
        "PushdownState.cpp"
    )
    source_group("AI\\Pushdown Automata" FILES ${AI_Pushdown_Automata})

    set(AI_State_Machine
        "State.h"
        "StateMachine.h"  
        "StateMachine.cpp"
        "StateMachine.h"
        "StateTransition.h"
    )
    source_group("AI\\State Machine" FILES ${AI_State_Machine})

    set(AI_Pathfinding
        "NavigationGrid.h"
        "NavigationGrid.cpp"  
        "NavigationMesh.cpp"
        "NavigationMesh.h"
        "NavigationMap.h"
        "NavigationPath.h"
    )
    source_group("AI\\Pathfinding" FILES ${AI_Pathfinding})

    set(AI_Guard
        "GuardObject.h"
        "GuardObject.cpp"
    )
    source_group("AI\\Guard" FILES ${AI_Guard})

    set(AI_CCTV
        "CCTV.h"
        "CCTV.cpp"
    )
    source_group("AI\\CCTV" FILES ${AI_CCTV})

    set(Collision_Detection
        "AABBVolume.h"
        "CapsuleVolume.h"  
        "CapsuleVolume.cpp"
        "CollisionDetection.h"
        "CollisionDetection.cpp"
        "CollisionVolume.h"
        "OBBVolume.h"
        "QuadTree.h"
        "QuadTree.cpp"
        "Ray.h"
        "SphereVolume.h"
    )
    source_group("Collision Detection" FILES ${Collision_Detection})

    set(Level_Creation
        "JsonParser.h"
        "JsonParser.cpp"
        "Level.h"
        "Level.cpp"
        "LevelEnums.h"
        "LevelEnums.cpp"
        "Room.h"
        "Room.cpp"
    )
    source_group("Level\\Level Creation" FILES ${Level_Creation})

    set(NavMesh
        "RecastBuilder.h"
        "RecastBuilder.cpp"
    )
    source_group("Level\\NavMesh" FILES ${NavMesh})

    set(Level_Objects
        "Door.h"
        "Door.cpp"
        "InteractableDoor.h"
        "InteractableDoor.cpp"
        "Interactable.h"
        "PrisonDoor.h"
        "PrisonDoor.cpp"
        "Helipad.h"
        "Helipad.cpp"
        "Vent.h"
        "Vent.cpp"
        "Interactable.h"
        "PointGameObject.h"
        "PointGameObject.cpp"
    )
    source_group("Level\\Level Objects" FILES ${Level_Objects})

    set(Networking
        "GameClient.h"  
        "GameClient.cpp"
        "GameServer.h"
        "GameServer.cpp"
        "NetworkBase.h"
        "NetworkBase.cpp"
        "NetworkObject.h"
        "NetworkObject.cpp"
        "NetworkState.h"
        "NetworkState.cpp"
    )
    source_group("Networking" FILES ${Networking})

    set(Physics
        "constraint.h"  
        "constraint.h"  
        "PositionConstraint.cpp"
        "PositionConstraint.h"
        "OrientationConstraint.cpp"
        "OrientationConstraint.h"
        "PhysicsObject.cpp"
        "PhysicsObject.h"
        "PhysicsSystem.cpp"
        "PhysicsSystem.h"
    )
    source_group("Physics" FILES ${Physics})

    set(Header_Files
        "Debug.h"
        "GameObject.h"
        "PlayerObject.h"
        "GameWorld.h"
        "RenderObject.h"
        "Transform.h"
        "AnimationObject.h"
        "AnimationSystem.h"
    )
    source_group("Header Files" FILES ${Header_Files})

    set(Source_Files
        "Debug.cpp"
        "GameObject.cpp"
        "PlayerObject.cpp"
        "GameWorld.cpp"
        "RenderObject.cpp"
        "Transform.cpp"
        "AnimationObject.cpp"
        "AnimationSystem.cpp"
    )
    source_group("Source Files" FILES ${Source_Files})

    #set(enet_Files
        #"./enet/callbacks.h"
        #"./enet/callbacks.c"
        #"./enet/list.h"
        #"./enet/list.c"
        #"./enet/protocol.h"
        #"./enet/protocol.c"
        #"./enet/win32.h"
        #"./enet/win32.c"

        #"./enet/enet.h"
        #"./enet/time.h"
        #"./enet/types.h"
        #"./enet/utility.h"

        #"./enet/compress.c"
        #"./enet/host.c"
        #"./enet/packet.c"
        #"./enet/peer.c"
    #)
    #source_group("eNet" FILES ${enet_Files})

    set(ALL_FILES
        ${Header_Files}
        ${Source_Files}
        ${AI_Behaviour_Tree}
        ${AI_Pushdown_Automata}
        ${AI_State_Machine}
        ${AI_Pathfinding}
        ${Collision_Detection}
        ${Level_Creation}
        ${NavMesh}
        ${Level_Objects}
        ${Networking}
        ${Physics}
        ${enet_Files}
        ${AI_Guard}
        ${AI_CCTV}
    )

    set_source_files_properties(${ALL_FILES} PROPERTIES LANGUAGE CXX)

    ################################################################################
    # Target
    ################################################################################
    add_library(${PROJECT_NAME} STATIC ${ALL_FILES})
    #use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
    set(ROOT_NAMESPACE CSC8503CoreClasses)
    ################################################################################
    # Compile definitions
    ################################################################################
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "UNICODE"
        "_UNICODE"
        "WIN32_LEAN_AND_MEAN"
        "_WINSOCK_DEPRECATED_NO_WARNINGS"
        "NOMINMAX"
    )

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


    ################################################################################
    # Dependencies
    ################################################################################
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "NCLCoreClasses"
        "ws2_32.lib"
        )
    include_directories("../NCLCoreClasses/")
    include_directories("../PS5Core")
    include_directories("./")
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC NCLCoreClasses)
    target_link_libraries(${PROJECT_NAME} LINK_PUBLIC PS5Core)

    if(MSVC)
        target_link_libraries(${PROJECT_NAME} PRIVATE "ws2_32.lib")
    endif()
endfunction()

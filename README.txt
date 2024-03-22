
TO SET UP THIS PROJECT TO RUN ON PC

1) delete the CMakeCache.txt file in CSC8508-2023
2) run cmake (delete the cache here, just to be sure) on the root directory, CSC8503-2023


TO SET UP THIS PROJECT FOR THE PS5

1) delete the CMakeCache.txt file in CSC8508-2023
2) run the Generate.bat file in the root directory, CSC8508-2023

PEOPLE'S INDIVIDUAL CONTRIBUTIONS (BROADLY)

Z. Chen - animation system, ui system, including interactive/dynamic elements like the suspicion meter and the in-game UI
S.E. Degirmenci - the networking system, across multiple PCs, and multithreaded 
A.J.R. Fall - level editor, loading system, multithreading, physics optimisations
K. Nicolaou - inventory system, items & buff system, suspicion system (local, global, and location-based)
O. Perrin - AI system - behaviour trees, pathfinding using a nav-mesh powered by Recast & Detour
B. Schwarz - renderer for PC & PS5 - deferred rendering, instancing, UBOs
E.D. Squire - player controller, friction, elasticity, and PS5 porting 
Z. Wang - dynamic minimap, showing wall and item positions
Y. Zhu - sound system, powered by FMod, both 2D and 3D, and across the network
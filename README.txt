All code that is mine should be marked with an author tag and a brief description of what it does. Look at last commit to see all notations added.

The following files are completely mine:

- LevelGenerator.h
- LevelGenerator.cpp

The following files do not contain any code that has been written or editted by myself:

- Assets
- CMake
- CMakeFiles
- NCLCoreClasses
- OpenGL rendering
- VulkanRendering

CMAKE Instructions for getting code on own machine:

1: Install CMakeLinks to an external site. (if you can), or extract the zipLinks to an external site.to a folder somewhere - the exe will be in the /bin/ sub folder. 

2: Extract the CSC8503 zip somewhere on your C: drive. There seems to be some issues with .pdb files on the H drive that are causing yet further issues. If you have admin rights then you can make a new folder on C:\, if you don't you can use one of the existing folders that you have rights to.

3: Open up CMakeLists.txt in notepad, and change the line set(CMAKE_CXX_STANDARD 23) to set(CMAKE_CXX_STANDARD 20) - this fixes an error later on.

4: Open up CMake, and set the 'Where is the source code' and 'Where to build the binaries' folder to the folder you extracted the code to. 

5: Go to File, and then select Delete Cache.

6: Press the Configure button in the bottom left of CMake. Leave the dialog box options alone and press Finish.

7: Press Generate. If you want to take a look at a different rendering API, select the USE_VULKAN tickbox, and then press Generate. 

8: Press Open Project. 

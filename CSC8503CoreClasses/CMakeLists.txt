set(PROJECT_NAME CSC8503CoreClasses)

include("CMakePC.cmake")
include("CMakePS5.cmake")

# PC CMake file
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    Create_CSC8503CoreClasses_Files_PC()
endif()

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Prospero")
    Create_CSC8503CoreClasses_Files_PS5()
endif()
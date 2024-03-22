set(PROJECT_NAME EntryPoint)

include("CMakePS5.cmake")
include("CMakePC.cmake")

# PC CMake file
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    Create_PC_EntryPoint_Files()
endif()

# PS5 Cmake file

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Prospero")
    Create_PS5_EntryPoint_Files()
endif()
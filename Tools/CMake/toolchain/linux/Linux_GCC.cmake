message(STATUS "Linux Gcc Toolchain\n***************")
set(CMAKE_CONFIGURATION_TYPES Debug Profile Release)
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "Reset the configurations to what we need" FORCE)

set(BUILD_CPU_ARCHITECTURE x64)
set(BUILD_PLATFORM Linux64)
set(LINUX 1)
set(LINUX64 1)


set(OUTPUT_DIRECTORY_NAME "linux_x64_gcc")

#[[
set(CMAKE_C_COMPILER gcc-4.9)
set(CMAKE_CXX_COMPILER g++-4.9)
]]

add_definitions(-DLINUX64)

message(STATUS "BUILD_CPU_ARCHITECTURE = ${BUILD_CPU_ARCHITECTURE}")

#include ("${CMAKE_CURRENT_LIST_DIR}/../../CRYENGINE-GCC.cmake")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLINUX -D__linux__ -DOPENGL" CACHE STRING "C Common Flags" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DLINUX -D__linux__ -DOPENGL" CACHE STRING "C++ Common Flags" FORCE)


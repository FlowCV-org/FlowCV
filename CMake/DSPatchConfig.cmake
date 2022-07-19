message(STATUS "Adding DSPatch CMake Config")

# DSPatch Library
include_directories(${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/dspatch/include)
include_directories(${CMAKE_SOURCE_DIR}/Managers/Common)
include_directories(${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/dspatch/src)
file(GLOB_RECURSE DSPatch_SRC ${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/dspatch/src/*.cpp)

# DSPatch Library
message(STATUS "Adding DSPatch CMake Config")

include_directories(${FLOWCV_PROJ_DIR}/third-party/dspatch/include)
include_directories(${FLOWCV_PROJ_DIR}/third-party/dspatch/src)
file(GLOB_RECURSE DSPatch_SRC ${FLOWCV_PROJ_DIR}/third-party/dspatch/src/*.cpp)

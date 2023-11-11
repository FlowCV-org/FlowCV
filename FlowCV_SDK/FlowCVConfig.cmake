# ======================================================
# Version variables:
# ======================================================
set(FLOWCV_VERSION 1.1.0)
set(FLOWCV_VERSION_MAJOR 1)
set(FLOWCV_VERSION_MINOR 1)
set(FLOWCV_VERSION_PATCH 0)
set(FLOWCV_VERSION_TWEAK 0)
set(FLOWCV_VERSION_STATUS "")

set(FLOWCV_PROJ_DIR ${CMAKE_CURRENT_LIST_DIR})

option(USE_LOCAL_OPENCV_PACKAGE "Use locally OpenCV Package" ON)

include(${FLOWCV_PROJ_DIR}/CMake/ImGuiConfig.cmake)
include(${FLOWCV_PROJ_DIR}/CMake/FlowCVConfig.cmake)
include(${FLOWCV_PROJ_DIR}/CMake/DSPatchConfig.cmake)
include(${FLOWCV_PROJ_DIR}/CMake/OpenCvConfig.cmake)
include(${FLOWCV_PROJ_DIR}/CMake/spdlogSetupConfig.cmake)
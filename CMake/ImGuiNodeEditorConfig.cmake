message(STATUS "Adding ImGUI Node Editor CMake Config")

set(IMGUI_NODE_EDITOR_DIR ${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/node-editor)
include_directories(${IMGUI_NODE_EDITOR_DIR})
include_directories(${CMAKE_SOURCE_DIR}/Editor_UI/Common)
include_directories(${IMGUI_NODE_EDITOR_DIR}/external/stb_image)
include_directories(${IMGUI_NODE_EDITOR_DIR}/external/ScopeGuard)
add_subdirectory(${IMGUI_NODE_EDITOR_DIR}/external/ScopeGuard)
add_subdirectory(${IMGUI_NODE_EDITOR_DIR}/external/stb_image)

file(GLOB IMGUI_NODE_EDITOR_SRC ${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/node-editor/*.cpp)

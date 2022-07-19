#ImGUI Config
message(STATUS "Adding ImGUI CMake Config")

# Third Party
include_directories(${FLOWCV_PROJ_DIR}/third-party/imgui_wrapper)
list(APPEND IMGUI_SRC "${FLOWCV_PROJ_DIR}/third-party/imgui_wrapper/imgui_instance_helper.cpp")

# ImGui
set(IMGUI_DIR ${FLOWCV_PROJ_DIR}/third-party/imgui)
include_directories(${IMGUI_DIR})
include_directories(${IMGUI_DIR}/backends)
list(APPEND IMGUI_SRC "${IMGUI_DIR}/imgui.cpp")
list(APPEND IMGUI_SRC "${IMGUI_DIR}/imgui_draw.cpp")
list(APPEND IMGUI_SRC "${IMGUI_DIR}/imgui_widgets.cpp")
list(APPEND IMGUI_SRC "${IMGUI_DIR}/imgui_tables.cpp")


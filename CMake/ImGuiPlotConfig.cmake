message(STATUS "Adding ImGUI Plot CMake Config")

add_compile_definitions(IMPLOT_ENABLED)
set(IMGUI_PLOT_DIR ${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/implot)
include_directories(${IMGUI_PLOT_DIR})

list(APPEND IMGUI_WRAPPER_SRC "${IMGUI_PLOT_DIR}/implot.cpp")
list(APPEND IMGUI_WRAPPER_SRC "${IMGUI_PLOT_DIR}/implot_items.cpp")

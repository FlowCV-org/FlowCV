project(Plugin_Name)

add_library(
        ${PROJECT_NAME} SHARED
        source_file.cpp
        ${IMGUI_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
        ${FlowCV_SRC}
)

target_link_libraries(
        ${PROJECT_NAME}
        ${IMGUI_LIBS}
        ${OpenCV_LIBS}
)

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
        SUFFIX ".fp"
        )

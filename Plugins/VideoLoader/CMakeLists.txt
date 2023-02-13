project(VideoLoader)

add_library(
        ${PROJECT_NAME} SHARED
        video_loader.cpp
        ${FlowCV_SRC}
        ${DSPatch_SRC}
        ${IMGUI_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
)

target_link_libraries(
        ${PROJECT_NAME}
        ${IMGUI_LIBS}
        ${OpenCV_LIBS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".fp")
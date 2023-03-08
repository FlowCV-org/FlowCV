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

if(WIN32)
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
        SUFFIX ".fp"
        )
elseif(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
            SUFFIX ".fp"
            INSTALL_RPATH "${ORIGIN}"
            BUILD_WITH_INSTALL_RPATH ON
            )
elseif(APPLE)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Plugins"
            SUFFIX ".fp"
            INSTALL_NAME_DIR "${ORIGIN}"
            BUILD_WITH_INSTALL_NAME_DIR ON
            )
endif()

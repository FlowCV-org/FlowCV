PROJECT(FlowCV_Dataflow_Test)

include_directories(${CMAKE_SOURCE_DIR}/Internal_Nodes)
include_directories(${CMAKE_SOURCE_DIR}/Managers)

file(GLOB_RECURSE INTERNAL_SRC ${CMAKE_SOURCE_DIR}/Internal_Nodes/*.cpp)
file(GLOB_RECURSE MANAGER_SRC ${CMAKE_SOURCE_DIR}/Managers/*.cpp)

if (UNIX AND NOT APPLE)
    add_link_options(-fno-pie -no-pie)
    set(STB_IMAGE_LIB stb_image)
endif()

add_executable(${PROJECT_NAME} flowcv_dataflow_test.cpp
        ${IMGUI_SRC}
        ${FlowCV_SRC}
        ${DSPatch_SRC}
        ${IMGUI_WRAPPER_SRC}
        ${IMGUI_OPENCV_SRC}
        ${INTERNAL_SRC}
        ${MANAGER_SRC}
        )

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
            ${IMGUI_LIBS}
            ${OpenCV_LIBS}
            )
else()
    target_link_libraries(${PROJECT_NAME}
            ${IMGUI_LIBS}
            ${OpenCV_LIBS}
            ${STB_IMAGE_LIB}
            pthread
            )
endif()

set_target_properties(${PROJECT_NAME}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        )

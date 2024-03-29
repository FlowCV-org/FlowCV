project(VideoCapture)

LIST(APPEND CAM_ENUM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/camera_enumerator.cpp)

if(WIN32)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win)
    LIST(APPEND CAM_ENUM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/win/Camera_MSMF.cpp)
endif()

if(UNIX AND NOT APPLE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/linux)
    LIST(APPEND CAM_ENUM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/linux/Camera_V4L.cpp)
endif()

if(APPLE)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/macos)
    LIST(APPEND CAM_ENUM_SRC ${CMAKE_CURRENT_SOURCE_DIR}/macos/Camera_MacOS.cpp)
    find_library(IOKIT_LIBRARY IOKit)
    SET(SYS_LIBS ${IOKIT_LIBRARY})
endif()

add_library(
    ${PROJECT_NAME} SHARED
    video_capture.cpp
    ${IMGUI_SRC}
    ${DSPatch_SRC}
    ${IMGUI_WRAPPER_SRC}
    ${IMGUI_OPENCV_SRC}
    ${FlowCV_SRC}
    ${CAM_ENUM_SRC}
)

target_link_libraries(
    ${PROJECT_NAME}
    ${IMGUI_LIBS}
    ${OpenCV_LIBS}
    ${SYS_LIBS}
    spdlog::spdlog
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

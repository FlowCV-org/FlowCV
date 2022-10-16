PROJECT(FlowCV_Engine)

include_directories(${CMAKE_SOURCE_DIR}/Internal_Nodes)
include_directories(${CMAKE_SOURCE_DIR}/Editor_UI/Common)
include_directories(${CMAKE_SOURCE_DIR}/Managers)
include_directories("${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/tclap/include")

file(GLOB_RECURSE INTERNAL_SRC ${CMAKE_SOURCE_DIR}/Internal_Nodes/*.cpp)
file(GLOB_RECURSE MANAGER_SRC ${CMAKE_SOURCE_DIR}/Managers/*.cpp)

if (UNIX AND NOT APPLE)
    add_link_options(-fno-pie -no-pie -Wl,--disable-new-dtags)
    set(STB_IMAGE_LIB stb_image)
endif()

add_executable(${PROJECT_NAME} headless_process_engine.cpp
        ${CMAKE_SOURCE_DIR}/Editor_UI/Common/app_settings.cpp
        ${IMGUI_SRC}
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

if(WIN32)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            )
elseif(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            INSTALL_RPATH ${ORIGIN};./Plugins/OpenVino;./Plugins/RealSense;./Plugins/NDI
            BUILD_WITH_INSTALL_RPATH ON
            )
elseif(APPLE)
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
            INSTALL_NAME_DIR ${ORIGIN};./Plugins/OpenVino;./Plugins/RealSense;./Plugins/NDI
            BUILD_WITH_INSTALL_NAME_DIR ON
            )
endif()

# OpenCV Configuration
message(STATUS "Adding OpenCV CMake Config")

if (USE_LOCAL_OPENCV_PACKAGE)
    find_package(OpenCV)
    if(OpenCV_FOUND)
        message(STATUS "OpenCV_VERSION: ${OpenCV_VERSION}, OpenCV_DIR: ${OpenCV_DIR}")
    else()
        message(WARNING "OpenCV Package NOT Found, download it")
        set(USE_LOCAL_OPENCV_PACKAGE OFF)
    endif()
endif()

if (NOT USE_LOCAL_OPENCV_PACKAGE)
    if (WIN32)
        if (NOT "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
            message(FATAL_ERROR "Only 64-bit supported on Windows")
        endif()

        set(OPENCV_FILE_NAME_RELEASE "opencv-4.5.5-openvino-dldt-2021.4.2-vc16-avx2.7z")

        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/opencv*")
            # Download OpenCV
            if ((DEFINED CMAKE_BUILD_TYPE) AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
                message(WARNING "See https://github.com/FlowCV-org/FlowCV/issues/8")
            elseif(NOT EXISTS "${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/opencv/build/lib/opencv_core455.lib")
                message(STATUS "Downloading OpenCV Package (Release)")
                file(DOWNLOAD https://github.com/opencv/opencv/releases/download/4.5.5/opencv-4.5.5-openvino-dldt-2021.4.2-vc16-avx2.7z 
                    "${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/${OPENCV_FILE_NAME_RELEASE}"
                    EXPECTED_HASH SHA256=f76c83db33815ce27144c6b20ed37e8f0b4ae199ad9b3a0291ccfcf7b0fb2703
                )
                message(STATUS "Download Complete")
            else()
                message(STATUS "OpenCV Package Already Exists In ${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/opencv")
            endif()
        endif()

        # Extract OpenCV
        if(NOT EXISTS "${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/opencv")
            message(STATUS "Extracting Files...")
            file(ARCHIVE_EXTRACT INPUT "${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/${OPENCV_FILE_NAME_RELEASE}" DESTINATION "${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/")
            message(STATUS "Files Extracted")
            message(STATUS "Removing Archive")
            file(REMOVE "${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/${OPENCV_FILE_NAME_RELEASE}")
        endif()

        # Set OpenCV_DIR
        set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/FlowCV_SDK/third-party/opencv/build/cmake")
        find_package(OpenCV REQUIRED PATHS ${OpenCV_DIR})

        # Copy OpenCV DLL
        file(GLOB files "${OpenCV_DIR}/../bin/*.dll")
        foreach(file_cv ${files})
            file(COPY "${file_cv}" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/")
        endforeach()
    else()
        message(FATAL_ERROR "No OpenCV Detected Please Install OpenCV System Package")
    endif()
endif()
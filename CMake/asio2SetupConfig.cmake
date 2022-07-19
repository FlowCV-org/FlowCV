message(STATUS "Configuring ASIO2")

include(FetchContent)

FetchContent_Declare(
        asio2
        GIT_REPOSITORY https://github.com/zhllxt/asio2
        GIT_TAG d18ad5426c9ba23dde06a6177619d0196c5993ee
)

FetchContent_GetProperties(asio2)

if (NOT asio2_POPULATED)
    FetchContent_Populate(asio2)

    set(ASIO2_ROOT_DIR ${asio2_SOURCE_DIR})

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ASIO2_LIBS_DIR ${ASIO2_ROOT_DIR}/lib/x64)
        set(ASIO2_EXES_DIR ${ASIO2_ROOT_DIR}/example/bin/x64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(ASIO2_LIBS_DIR ${ASIO2_ROOT_DIR}/lib/x86)
        set(ASIO2_EXES_DIR ${ASIO2_ROOT_DIR}/example/bin/x86)
    endif()

    IF (CMAKE_SYSTEM_NAME MATCHES "Linux")
        message("if the [unrecognized command line option '-std=c++17'] error occurs, attempt to execute this command: export CC=/usr/local/bin/gcc; export CXX=/usr/local/bin/g++;")
        set(OPENSSL_LIBS libssl.a libcrypto.a)
        set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
    ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Windows")
        set(OPENSSL_LIBS "libssl.lib;libcrypto.lib;Crypt32.lib;")
        set(GENERAL_LIBS "ws2_32;mswsock;")
    ELSEIF (CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(OPENSSL_LIBS libssl.a libcrypto.a)
        set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
    ELSEIF (CMAKE_SYSTEM_NAME MATCHES "Darwin")
        set(OPENSSL_LIBS libssl.a libcrypto.a)
        set(GENERAL_LIBS -lpthread -ldl)
    ELSE ()
        set(OPENSSL_LIBS libssl.a libcrypto.a)
        set(GENERAL_LIBS -lpthread -lrt -ldl stdc++fs)
    ENDIF (CMAKE_SYSTEM_NAME MATCHES "Linux")

    if (MSVC)
        set (CMAKE_VERBOSE_MAKEFILE FALSE)

        #add_definitions (
        #    -D_WIN32_WINNT=0x0601
        #    -D_SCL_SECURE_NO_WARNINGS=1
        #    -D_CRT_SECURE_NO_WARNINGS=1
        #    -D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
        #    -D_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
        #)

        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            add_compile_options(
                    /bigobj       # large object file format
                    #/permissive-  # strict C++
                    #/wd4503      # decorated name length exceeded, name was truncated
                    /W4           # enable all warnings
                    /JMC
            )

            set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MTd")
            set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /Zi /Ob2 /Oi /Ot /MT")

            set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
        else()
            add_compile_options(
                    /bigobj       # large object file format
                    #/permissive-  # strict C++
                    #/wd4503      # decorated name length exceeded, name was truncated
                    /W4           # enable all warnings
                    /MP           # Multi-processor compilation
                    /JMC
            )

            set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /std:c++17 /MTd /ZI")
            set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /std:c++17 /Zi /Ob2 /Oi /Ot /GL /MT")

            set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
            set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG:incremental")
        endif ()
    else()
        set (THREADS_PREFER_PTHREAD_FLAG ON)
        find_package (Threads)

        set( CMAKE_CXX_FLAGS
                "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter")

        #set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -ggdb")

        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wrange-loop-analysis")
        endif ()
    endif()


    # https://stackoverflow.com/questions/44705845/file-too-big-compiling-on-cygwin-g
    if(MINGW OR CYGWIN)
        add_definitions(-O3)
    endif()

    include_directories (${ASIO2_ROOT_DIR}/3rd)
    include_directories (${ASIO2_ROOT_DIR}/include)

    file (GLOB_RECURSE  ASIO2_FILES  ${ASIO2_ROOT_DIR}/include/asio2/*.*)
    file (GLOB_RECURSE   ASIO_FILES  ${ASIO2_ROOT_DIR}/3rd/asio/*.*)
    file (GLOB_RECURSE CEREAL_FILES  ${ASIO2_ROOT_DIR}/3rd/cereal/*.*)

    # exclude : asio/impl/src.cpp
    list(REMOVE_ITEM ASIO_FILES ${ASIO2_ROOT_DIR}/3rd/asio/impl/src.cpp)

endif()
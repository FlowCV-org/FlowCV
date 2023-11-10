message(STATUS "Configuring spdlog")

# https://github.com/gabime/spdlog/issues/2758
include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY "https://github.com/gabime/spdlog.git"
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/_deps/spdlog-src/include")

add_compile_options(-DSPDLOG_COMPILED_LIB)
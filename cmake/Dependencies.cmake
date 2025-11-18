# Dependencies.cmake
# Manages external dependencies for TweakXL-macOS

include(FetchContent)

# Set FetchContent options
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ============================================================================
# yaml-cpp - YAML parsing
# ============================================================================
message(STATUS "Fetching yaml-cpp...")
FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 0.8.0
    GIT_SHALLOW TRUE
)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(yaml-cpp)

# ============================================================================
# PEGTL - Parsing Expression Grammar Template Library
# ============================================================================
message(STATUS "Fetching PEGTL...")
FetchContent_Declare(
    pegtl
    GIT_REPOSITORY https://github.com/taocpp/PEGTL.git
    GIT_TAG 3.2.7
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(pegtl)

# ============================================================================
# spdlog - Fast C++ logging library
# ============================================================================
message(STATUS "Fetching spdlog...")
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
    GIT_SHALLOW TRUE
)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(spdlog)

# ============================================================================
# CLI11 - Command line parser
# ============================================================================
message(STATUS "Fetching CLI11...")
FetchContent_Declare(
    cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.3.2
    GIT_SHALLOW TRUE
)
set(CLI11_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CLI11_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(cli11)

# ============================================================================
# Catch2 - Unit testing framework
# ============================================================================
if(BUILD_TESTS)
    message(STATUS "Fetching Catch2...")
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

message(STATUS "All dependencies fetched successfully")

cmake_minimum_required(VERSION 3.31.6)
project(github-activity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
# Disable CPR's internal tests and landmarks so your build is fast
set(CPR_BUILD_TESTS OFF CACHE INTERNAL "")
set(CPR_BUILD_TESTS_SSL OFF CACHE INTERNAL "")

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG 1.14.2
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)

FetchContent_MakeAvailable(cpr json stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})

add_executable(github-activity src/main.cpp)

target_link_libraries(github-activity PRIVATE cpr::cpr nlohmann_json::nlohmann_json stb)

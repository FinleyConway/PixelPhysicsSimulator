cmake_minimum_required(VERSION 3.24...3.30)
project(SandSimulator)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
include(CTest)

# Boost
find_package(Boost REQUIRED)

# Raylib
set(RAYLIB_VERSION 5.5)
FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
)
set(GLFW_BUILD_WAYLAND ON)
set(GLFW_BUILD_X11 OFF)
FetchContent_MakeAvailable(raylib)

# Catch2
set(CATCH2_VERSION v3.8.1)
FetchContent_Declare(
    catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/${CATCH2_VERSION}.zip
)
FetchContent_MakeAvailable(catch2)

# Main Executable
add_executable(SandSimulator)
add_subdirectory(src)

target_include_directories(SandSimulator PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(SandSimulator PRIVATE raylib)

set_target_properties(SandSimulator PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
    VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:SandSimulator>
)

# Tests
add_subdirectory(test)
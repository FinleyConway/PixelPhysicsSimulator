cmake_minimum_required(VERSION 3.24...3.30)
project(SandSimulator)

include(FetchContent)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Raylib
set(RAYLIB_VERSION 5.5)
FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
    FIND_PACKAGE_ARGS
)
set(GLFW_BUILD_WAYLAND ON)
set(GLFW_BUILD_X11 OFF)
FetchContent_MakeAvailable(raylib)

# Add boost
find_package(Boost REQUIRED)

# Our Project
add_executable(${PROJECT_NAME})
add_subdirectory(src)

target_include_directories(SandSimulator PRIVATE ${Boost_INCLUDE_DIRS})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)

target_link_libraries(${PROJECT_NAME} raylib)
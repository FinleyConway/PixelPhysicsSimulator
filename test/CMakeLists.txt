file(GLOB TEST_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(SandSimulatorTests ${TEST_SOURCES})

target_link_libraries(SandSimulatorTests PRIVATE raylib Catch2::Catch2WithMain)
target_include_directories(SandSimulatorTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/test
)

include(Catch)
catch_discover_tests(SandSimulatorTests)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS *.cpp)
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*main\\.cpp") # really hacky
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS *.h)

add_library(SandSimulatorLib STATIC ${SOURCE_FILES} ${HEADER_FILES})

target_include_directories(SandSimulatorLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(SandSimulatorLib PUBLIC raylib)
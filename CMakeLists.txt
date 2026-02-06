cmake_minimum_required(VERSION 3.26)

project(SLICK_GAME)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_EXAMPLES OFF)

if(EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(PLATFORM web CACHE STRING "Run on Web")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

add_subdirectory(vendor/raylib-5.5)
add_subdirectory(vendor/raygui-4.0)
add_executable(SLICK_GAME)

target_sources(SLICK_GAME
    PRIVATE
      src/main.cpp
)

target_include_directories(SLICK_GAME
    PRIVATE
    "${CMAKE_SOURCE_DIR}/vendor/raylib-5.5/src",
    "${CMAKE_SOURCE_DIR}/vendor/raygui-4.0/src"
)

target_link_libraries(SLICK_GAME
  PRIVATE
    raylib
    raygui
)

if(EMSCRIPTEN)
TARGET_LINK_OPTIONS(SLICK_GAME PRIVATE
    "-sUSE_GLFW=3"
    "-sWASM=1"
    "-sALLOW_MEMORY_GROWTH =1"
    "-sASYNCIFY"
    "--preload-file" "${CMAKE_SOURCE_DIR}/LdkLevels@LdkLevels"
)
endif()

cmake_minimum_required(VERSION 3.26)

project(SLICK_GAME)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_EXAMPLES OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(vendor/raylib-5.5)

add_executable(SLICK_GAME)

target_sources(SLICK_GAME
    PRIVATE
      src/main.cpp
)

target_include_directories(SLICK_GAME 
    PRIVATE 
    "${CMAKE_SOURCE_DIR}/vendor/raylib-5.5/src"
)

target_link_libraries(SLICK_GAME
  PRIVATE
    raylib
)

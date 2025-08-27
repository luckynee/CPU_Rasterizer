cmake_minimum_required(VERSION 3.14)

project(CPU_Rasterizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Find required packages
find_package(SDL2 QUIET)

if(NOT SDL2_FOUND)
    message(STATUS "SDL2 not found, downloading from source...")
    # Download and build SDL2
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.0  # Use stable version
    )
    
    # Configure SDL2 options
    set(SDL2_DISABLE_INSTALL OFF CACHE BOOL "" FORCE)
    set(SDL_SHARED ON CACHE BOOL "" FORCE)
    set(SDL_STATIC OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL2)
    set(SDL_TARGET SDL2::SDL2)
else()
    message(STATUS "Using system SDL2")
    set(SDL_TARGET SDL2::SDL2)
endif()

add_executable(${PROJECT_NAME} ${MY_SOURCES})

# Link SDL2 libraries
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL_TARGET})

# Include Dir
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Compiler-specific option
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
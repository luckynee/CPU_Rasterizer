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

# Main application target
#add_executable(${PROJECT_NAME} ${MY_SOURCES})
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/core/*.cpp")

add_executable(${PROJECT_NAME} src/core/main.cpp ${CORE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL_TARGET})

# Demo targets
file(GLOB_RECURSE TERRAIN_DEMO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/demos/*.cpp")
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/core/main.cpp")
add_executable(terrain_demo src/demos/demos_main.cpp ${TERRAIN_DEMO_SOURCES} ${CORE_SOURCES})

target_include_directories(terrain_demo PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/demos"
)
target_link_libraries(terrain_demo PRIVATE ${SDL_TARGET})

# Optimization flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

#
#   Note: Enable optimization flags for release builds
#   make -DCMAKE_BUILD_TYPE=Release build/
#

# Enable Optimisation in release
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O3 -ffast-math)
elseif(CMAKE_BUILD_TYPE STREQUAL "Profiling")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2 -pg -ffast-math)
    target_link_options(${PROJECT_NAME} PRIVATE -pg)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Optimization and warning flags for all targets
foreach(target IN LISTS ${PROJECT_NAME} terrain_demo)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(${target} PRIVATE -O3 -ffast-math)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Profiling")
        target_compile_options(${target} PRIVATE -O2 -pg -ffast-math)
        target_link_options(${target} PRIVATE -pg)
    endif()
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endforeach()


# Suppress warnings in third-party code
set_source_files_properties(
    src/demos/vendor/OpenSimplexNoise.cpp
    PROPERTIES
    COMPILE_FLAGS "-Wno-unused-but-set-variable -Wno-unused-variable"
)
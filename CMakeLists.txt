cmake_minimum_required(VERSION 3.14)
project(CPU_Rasterizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# -------------------- SDL2 --------------------
find_package(SDL2 QUIET)

if(NOT SDL2_FOUND)
  message(STATUS "SDL2 not found, downloading from source...")
  FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.0
  )
  set(SDL2_DISABLE_INSTALL OFF CACHE BOOL "" FORCE)
  set(SDL_SHARED ON CACHE BOOL "" FORCE)
  set(SDL_STATIC OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(SDL2)
  set(SDL_TARGETS SDL2::SDL2 SDL2::SDL2main)
else()
  message(STATUS "Using system SDL2")
  set(SDL_TARGETS SDL2::SDL2 SDL2::SDL2main)
endif()

# SDL_config.h can land in different dirs depending on generator/config.
# Cover them all so the preprocessor always finds it.
set(SDL_CFG_BASE "${CMAKE_BINARY_DIR}/_deps/sdl2-build")
set(SDL_INCLUDE_CONFIG_DIRS
  "${SDL_CFG_BASE}/include-config"
  "${SDL_CFG_BASE}/include-config-$<CONFIG>"
  "${SDL_CFG_BASE}/include-config-$<LOWER_CASE:$<CONFIG>>"
  "${SDL_CFG_BASE}/include-config-$<UPPER_CASE:$<CONFIG>>"
  # Some builds copy config into the general include dir:
  "${SDL_CFG_BASE}/include"
)

# -------------------- Sources --------------------
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/core_engine/*.cpp")
file(GLOB_RECURSE DEMO_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/demos/*.cpp")

# -------------------- Main exe --------------------
add_executable(${PROJECT_NAME}
  src/core_engine/main.cpp
  ${CORE_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core_engine"
  ${SDL_INCLUDE_CONFIG_DIRS}
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL_TARGETS})

# -------------------- Terrain demo --------------------
# Reuse core sources but drop main.cpp to avoid duplicate mains
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/core_engine/main.cpp")

add_executable(terrain_demo
  src/demos/demos_main.cpp
  ${DEMO_SOURCES}
  ${CORE_SOURCES}
)

target_include_directories(terrain_demo PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/core_engine"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/demos"
  ${SDL_INCLUDE_CONFIG_DIRS}
)
target_link_libraries(terrain_demo PRIVATE ${SDL_TARGETS})

# -------------------- Build type & flags --------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

foreach(tgt ${PROJECT_NAME} terrain_demo)
  if(MSVC)
    target_compile_options(${tgt} PRIVATE /W4 /WX)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      target_compile_options(${tgt} PRIVATE /O2 /fp:fast)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Profiling")
      # MSVC profiling flags (if needed)
    endif()
  else()
    target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      target_compile_options(${tgt} PRIVATE -O3 -ffast-math)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Profiling")
      target_compile_options(${tgt} PRIVATE -O2 -pg -ffast-math)
      target_link_options(${tgt} PRIVATE -pg)
    endif()
  endif()
endforeach()
# Keep your vendor warning tweaks
set_source_files_properties(
  src/demos/vendor/OpenSimplexNoise.cpp
  PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable -Wno-unused-variable"
)


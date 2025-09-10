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
          GIT_TAG release-2.32.8 # Or latest stable release
  )
  set(SDL2_DISABLE_INSTALL OFF CACHE BOOL "" FORCE)
  set(SDL_SHARED ON CACHE BOOL "" FORCE)
  set(SDL_STATIC OFF CACHE BOOL "" FORCE)

  FetchContent_MakeAvailable(SDL2)

  set(SDL_TARGETS SDL2::SDL2main SDL2::SDL2)
else()
  message(STATUS "Using system SDL2")
  set(SDL_TARGETS SDL2::SDL2main SDL2::SDL2)
endif()

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
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${SDL_TARGETS})

# -------------------- Terrain demo --------------------
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
)
target_link_libraries(terrain_demo PRIVATE ${SDL_TARGETS})

# -------------------- Auto-copy DLL (for Windows) --------------------
if(WIN32 AND NOT CMAKE_CROSSCOMPILING)
  add_custom_command(
          TARGET ${PROJECT_NAME} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_BINARY_DIR}/_deps/sdl2-build/SDL2.dll"
          $<TARGET_FILE_DIR:${PROJECT_NAME}>
          COMMENT "Copying SDL2.dll to executable directory"
  )
  add_custom_command(
          TARGET terrain_demo POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E copy
          "${CMAKE_BINARY_DIR}/_deps/sdl2-build/SDL2.dll"
          $<TARGET_FILE_DIR:terrain_demo>
          COMMENT "Copying SDL2.dll to executable directory"
  )
endif()

# -------------------- Build type & flags --------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

foreach(tgt ${PROJECT_NAME} terrain_demo)
  if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      target_compile_options(${tgt} PRIVATE /O2 /fp:fast)
    endif()
  else()
    target_compile_options(${tgt} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
      target_compile_options(${tgt} PRIVATE -O3 -ffast-math)
    endif()
  endif()
endforeach()

# This is for ignoring warnings in specific vendor files
if(MSVC)
    set_source_files_properties(
        src/demos/vendor/OpenSimplexNoise.cpp
        PROPERTIES COMPILE_FLAGS "/wd4101 /wd4189"
    )
else()
    set_source_files_properties(
        src/demos/vendor/OpenSimplexNoise.cpp
        PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable -Wno-unused-variable"
    )
endif()

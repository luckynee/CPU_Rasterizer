# CPU_Rasterizer

A high-performance CPU-based rasterizer written in C++20, featuring terrain generation, custom shaders, and SDL2-based rendering. This repository contains two main executables:

- **CPU_Rasterizer**: The core engine demo.
- **terrain_demo**: A terrain generation and rendering demo.

## Prerequisites

- C++20 compatible compiler (e.g., GCC 13+)
- CMake 3.14+
- SDL2 (automatically downloaded if not found)

## Building the Project

1. **Clone the repository:**
   ```bash
   git clone https://github.com/luckynee/CPU_Rasterizer.git
   cd CPU_Rasterizer
   ```

2. **Configure the build:**
   - **Release build (default):**
     ```bash
     cmake -B build -DCMAKE_BUILD_TYPE=Release
     ```
   - **Debug build:**
     ```bash
     cmake -B build -DCMAKE_BUILD_TYPE=Debug
     ```
   - **Profiling build:**
     ```bash
     cmake -B build -DCMAKE_BUILD_TYPE=Profiling
     ```

3. **Build the executables:**
   ```bash
   cd build
   make CPU_Rasterizer -j
   make terrain_demo -j
   ```
   Or use the provided VS Code tasks ("Cmake Build").

## Running the Demos

- **Run the core engine demo:**
  ```bash
  ./CPU_Rasterizer
  ```

- **Run the terrain demo:**
  ```bash
  ./terrain_demo
  ```

## Project Structure

- `src/core_engine/` — Core rasterizer engine and utilities
- `src/demos/` — Demo applications (terrain, etc.)
- `resource/` — Models and textures

## Changing Build Type in VS Code

You can use the provided VS Code tasks to configure the build type:
- **Release:** Run the task "Cmake Configure Release"
- **Debug:** Run the task "Cmake Configure Debug"
- **Profiling:** Run the task "Cmake Configure Profiling"

After configuring, rebuild using "Cmake Build".

## Notes
- SDL2 will be automatically downloaded and built if not found on your system.
- All code is written in modern C++20 and uses custom math and shader classes.
- For troubleshooting, check the build output and ensure your compiler supports C++20.


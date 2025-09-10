# CPU_Rasterizer

![Language](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)
![Build](https://img.shields.io/badge/Build-CMake-red.svg)
![Library](https://img.shields.io/badge/Library-SDL2-brightgreen.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

A high-performance, cross-platform, CPU-based software renderer written from scratch in modern C++20. This project demonstrates rasterization techniques, including terrain generation from noise, model loading, and custom shader effects, with all windowing and input handled by SDL2.

## Features
-   **High-Performance Software Rendering**: Optimized for multi-core CPUs.
-   **Procedural Terrain Generation**: Creates dynamic landscapes using OpenSimplex noise.
-   **Custom Shader Support**: A flexible shader system for implementing custom lighting and color effects.
-   **Model and Texture Loading**: Supports `.obj` for 3D models and a custom `.bytes` format for textures.
-   **Cross-Platform**: Uses CMake to build and run on Windows, macOS, and Linux.
-   **Modern C++20**: Leverages modern C++ features for clean, efficient, and maintainable code.

## Prerequisites
-   **C++20 compatible compiler** (e.g., MSVC v143+, GCC 13+, Clang 16+)
-   **CMake 3.14+**
-   **SDL2**: Handled automatically via `FetchContent` if not found on your system.

---
## Building and Running
This project is configured with CMake and can be built using the command line or your favorite C++ IDE.

### Visual Studio (MSVC)
1.  Launch Visual Studio and select **"Open a local folder"**.
2.  Navigate to and select the root directory of the `CPU_Rasterizer` project.
3.  Visual Studio will automatically detect `CMakeLists.txt` and configure the project.
4.  Once configured, select the desired target (**CPU_Rasterizer** or **terrain_demo**) from the "Select Startup Item" dropdown in the main toolbar.
5.  Press the green **Run** button (▶️) to build and run the selected demo.

### CLion
1.  Launch CLion and select **File > Open**.
2.  Navigate to and select the project's root `CMakeLists.txt` file or the project folder.
3.  CLion will automatically load and sync the CMake project.
4.  In the top-right corner, select the desired target (**CPU_Rasterizer** or **terrain_demo**) from the configurations dropdown.
5.  Click the **Run** (▶️) or **Debug** (🐞) button to build and run.

### Visual Studio Code
1.  Make sure you have the **C++ Extension Pack** and **CMake Tools** extensions from Microsoft installed.
2.  Open the `CPU_Rasterizer` project folder in VS Code.
3.  A prompt will ask you to configure the project; select **Yes**. If it doesn't, open the Command Palette (`Ctrl+Shift+P`) and run **CMake: Configure**.
4.  Use the status bar at the bottom to:
    -   Select the build variant (`Debug`, `Release`, etc.).
    -   Select the active build target (`[CPU_Rasterizer]` or `[terrain_demo]`).
    -   Click the **Build** button or press `F7`.
    -   Click the **Run** button or press `Ctrl+F5`.

### Command Line (Universal)
1.  **Clone the repository:**
    ```bash
    git clone [https://github.com/luckynee/CPU_Rasterizer.git](https://github.com/luckynee/CPU_Rasterizer.git)
    cd CPU_Rasterizer
    ```
2.  **Configure the build** (creates a `build` directory):
    ```bash
    # For a Release build
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
    ```
3.  **Build the executables:**
    ```bash
    # This command works for Makefiles, Ninja, and MSBuild
    cmake --build build
    ```
4.  **Run the demos** (executables are inside the `build` directory):
    ```bash
    # Run the core engine demo
    ./build/CPU_Rasterizer
    
    # Run the terrain demo
    ./build/terrain_demo
    ```

---
## Supported Formats
-   **3D Models**: **`.obj`**. A standard, widely supported format for 3D geometry.
-   **Textures**: **`.bytes`**. A custom raw pixel data format used by this project for simplicity and fast loading.

## Project Structure
-   `src/core_engine/` — Core rasterizer engine, math utilities, and shader classes.
-   `src/demos/` — Source code for the demo applications.
-   `resource/` — Contains `.obj` models and `.bytes` textures.

## Troubleshooting
-   If you encounter build errors, ensure your compiler fully supports C++20.
-   On Windows, the `SDL2.dll` is automatically copied next to the executables. If the program fails to start with a DLL error, clean your build directory and rebuild.

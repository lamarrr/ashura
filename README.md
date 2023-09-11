# Ashura (ReGui) <img src="https://github.com/lamarrr/ashura/actions/workflows/msvc-windows-x64.yml/badge.svg">
Ashura is a GUI and Graphics Framework for use in High-performance GUI applications and games

## Features
- Concurrency & Multithreading
- Portability: Supports Windows, iOS, Linux, PS5? 
- Optimal resource usage
- Low mental overhead
- Flexible system and Widgets

## Documentation

## Examples

# Dependencies
- Install Vulkan SDK via apt, homebrew, https://vulkan.lunarg.com/sdk/home, or https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html. Make sure it is discoverable by CMake's find_package by having exported targets (i.e. VULKAN_SDK to point to the SDK's root directory)
- Install vcpkg via https://vcpkg.io/en/index.html
- Install MSVC, Clang, or GCC with C++20 support
- Install CMake (>= 3.1)
- Install [SDL 3](https://github.com/libsdl-org/SDL) with SDL_STATIC=ON to generate the SDL3-static target

## Building
- Build the project using (where ${PATH_TO_ASHURA} is replaced with the path to this project and ${PATH_TO_VCPKG} is replaced with the path to your vcpkg installation): 
```bash
cd ${PATH_TO_ASHURA} && mkdir build && cd build
```

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE="${PATH_TO_VCPKG}/scripts/buildsystems/vcpkg.cmake"
```

```bash
cmake --build .
```

## Contributing

## License

# Ashura (ReGui) <img src="https://github.com/lamarrr/workflows/msvc-2019-windows-x64.yml/badge.svg">
Ashura is an advanced 2D Retained-Mode GUI and Basic 3D Framework for use in GUI applications and games.

## Features
- Multithreaded:
- Portability: Supports Windows, iOS, Linux, Nintendo Switch
- Super Duper Fast
- Minimal resource usage
- Low mental overhead
- Flexible system and Widgets
- Usable for games and UI
- Ease of Integration of Accessibility features

## Documentation

## Examples


## Building
- Install Vulkan SDK via https://vulkan.lunarg.com/sdk/home or https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html whilst making sure it is discoverable by find_package by having exported targets
- Install vcpkg via https://vcpkg.io/en/index.html
- Install MSVC, Clang, or GCC with C++20 support
- Install CMake at least version 3.1

--- install SDL 3 with SDL_STATIC=ON

- Install dependencies via vcpkg:
```bash
vcpkg install freetype libpng[apng] harfbuzz fmt libwebp libjpeg-turbo libpng spdlog simdjson gtest libogg
```

- build the project using (where ${PATH_TO_ASHURA} is replaced with the path to this project and ${PATH_TO_VCPKG} is replaced with the path to your vcpkg installation): 
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
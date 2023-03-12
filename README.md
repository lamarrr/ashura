Ashura (temporary codename) is an advanced 2D GUI and Basic 3D Framework for use in GUI applications and games

# Goals
- Multithreaded
- Supports on win, ios, linux, ps5
- Portability
- Super Duper Fast
- Minimal resource usage
- Low mental overhead
- Flexible system and Widgets
- Usable for games and UI
- Ease of Integration of Accessibility features

# Building

- Install NASM via https://www.nasm.us/, apt-get on linux, homebrew on mac, or winget on windows
- Install vcpkg via https://vcpkg.io/en/index.html
- Install MSVC, Clang, or GCC with C++20 support
- Install CMake at least version 3.1

--- install SDL 3 with SDL_STATIC=ON

- Install dependencies via vcpkg:
```bash
vcpkg install vulkan freetype libpng[apng] harfbuzz fmt libwebp libjpeg-turbo libpng spdlog simdjson gtest libogg
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

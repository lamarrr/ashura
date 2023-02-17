- multithreaded
- works on win, ios, linux, ps5

What problems do we intend to solve?

- fast, easy and flexible advanced GUI
- multiplatform
- flexible for games and UI
- accessibility

What solutions do we provide to solve these problems?

# Building

```bash
vcpkg install vulkan sdl2[core,vulkan] freetype libpng[apng] harfbuzz fmt stb spdlog simdjson gtest
```

```bash
cd ${PATH_TO_ASHURA} && mkdir build && cd build
```

```bash
cmake .. -DCMAKE_TOOLCHAIN_FILE="${PATH_TO_VCPKG}/scripts/buildsystems/vcpkg.cmake"
```

```bash
cmake --build .
```

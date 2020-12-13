cmake_minimum_required(VERSION 3.1)

project(Valkyrie)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

execute_process(
  COMMAND git clone https://github.com/lamarrr/STX.git third_party/STX
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b 3.3.2 https://github.com/glfw/glfw.git third_party/glfw
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b v1.8.2 https://github.com/gabime/spdlog.git
          third_party/spdlog WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(third_party/STX)
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/spdlog)

find_package(Vulkan REQUIRED)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID}
                                                STREQUAL "GNU")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(valkyrie.bin src/vlk.cc)
target_link_libraries(valkyrie.bin stx glfw spdlog::spdlog ${Vulkan_LIBRARIES})
target_include_directories(
  valkyrie.bin PRIVATE ${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR}/include)

# todo: add GLSL compilation

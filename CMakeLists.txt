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
execute_process(
  COMMAND git clone -b 0.9.9.8 https://github.com/g-truc/glm.git third_party/glm
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
execute_process(
  COMMAND git clone -b master https://github.com/nothings/stb.git
          third_party/stb WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

          execute_process(
            COMMAND git clone -b release-1.10.0 https://github.com/google/googletest.git third_party/googletest WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})


set(GLFW_BUILD_EXAMPLES
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS
    OFF
    CACHE BOOL "" FORCE)
set(GLFW_INSTALL
    OFF
    CACHE BOOL "" FORCE)
set(STX_ENABLE_BACKTRACE
    ON
    CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF
CACHE BOOL "" FORCE)

add_subdirectory(third_party/STX)
add_subdirectory(third_party/glfw)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/glm)
add_subdirectory(third_party/googletest)

find_package(Vulkan REQUIRED)

add_executable(valkyrie.bin src/vlk.cc)
target_link_libraries(valkyrie.bin stx glfw spdlog::spdlog glm::glm
                      ${Vulkan_LIBRARIES})
target_include_directories(
  valkyrie.bin PRIVATE ${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR}/include
                       ${CMAKE_CURRENT_LIST_DIR}/third_party/stb)




add_executable(valkyrie_tests.bin tests/widget.cc)
target_link_libraries(valkyrie_tests.bin stx glfw spdlog::spdlog glm::glm
                      ${Vulkan_LIBRARIES} gtest gtest_main)

                      target_include_directories(
                        valkyrie_tests.bin PRIVATE ${Vulkan_INCLUDE_DIRS} ${CMAKE_CURRENT_LIST_DIR}/include
                                             ${CMAKE_CURRENT_LIST_DIR}/third_party/stb)


if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID}
                                                STREQUAL "GNU")
  target_compile_options(valkyrie.bin PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(valkyrie_tests.bin PRIVATE -Wall -Wextra -Wpedantic)
endif()
# todo: add GLSL compilation

function(add_spirv_shader TARGET_NAME IN_GLSL_FILE_PATH OUT_SPIRV_FILE)

  if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/${OUT_SPIRV_FILE})
    file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/${OUT_SPIRV_FILE})
  endif()

  add_custom_target(
    shader-${TARGET_NAME}
    glslangValidator -V ${IN_GLSL_FILE_PATH} -o
    ${CMAKE_CURRENT_BINARY_DIR}/${OUT_SPIRV_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

endfunction()

list(APPEND GLSL_TARGETS triangle)

foreach(GLSL_TARGET ${GLSL_TARGETS})
  add_spirv_shader(
    ${GLSL_TARGET}-frag shaders/${GLSL_TARGET}/${GLSL_TARGET}.frag
    ${GLSL_TARGET}.frag.spv)
  add_spirv_shader(
    ${GLSL_TARGET}-vert shaders/${GLSL_TARGET}/${GLSL_TARGET}.vert
    ${GLSL_TARGET}.vert.spv)
endforeach(GLSL_TARGET)

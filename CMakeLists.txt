cmake_minimum_required(VERSION 3.25)
project(SketchBook-Project VERSION 0.0.1)

set(CMAKE_BUILD_TYPE Debug)
set(VCPKG_TARGET_TRIPLET x64-windows)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(IMGUI_IMPL_DX12 OFF)
set(IMGUI_IMPL_DX11 OFF)
set(IMGUI_IMPL_DX10 OFF)
set(IMGUI_IMPL_SDL ON)
set(IMGUI_IMPL_VULKAN ON)
set(IMGUI_EXAMPLES OFF)
set(IMGUI_ENABLE_FREETYPE OFF)

find_package(Vulkan REQUIRED COMPONENTS glslc)
find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)

message(STATUS "vcpkg dir: ${sk_vcpkg_dir}")
message(STATUS "sdl2 dir: ${SDL2_DIR}")

set(SDL_TEST OFF)
set(SDL_DIRECTX OFF)
set(SDL_CPU_X64 ON)
set(SDL2_LIBRARIES "${SDL2_DIR}/lib/x64/SDL2.lib;${SDL2_DIR}/lib/x64/SDL2main.lib;")
set(SDL2_INCLUDE_DIRS ${SDL2_DIR}/include)

message(STATUS "sdl2 libs: ${SDL2_LIBRARIES}")
message(STATUS "sdl2 inclue: ${SDL2_INCLUDE_DIRS}")
message(STATUS "imgui dir: ${imgui_DIR}")

include(${sk_vcpkg_dir}/scripts/buildsystems/vcpkg.cmake)

INCLUDE_DIRECTORIES(${sk_vcpkg_dir}/installed/x64-windows/include/)
LINK_DIRECTORIES(${sk_vcpkg_dir}/installed/x64-windows/lib/)

LINK_DIRECTORIES(${SDL2_LIBRARIES})
INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS})

set (box2d_LIB ${box2d_DIR}/build/bin/Debug/box2d.lib)
set (box2d_INCLUDE ${box2d_DIR}/include)


list(APPEND CMAKE_PREFIX_PATH ${ReactPhysics3D_DIR}) 

set(ReactPhysics3D_LIB ${ReactPhysics3D_DIR}/Debug/reactphysics3d.lib)
set(ReactPhysics3D_INCLUDE ${ReactPhysics3D_DIR}/include)


include(FetchContent)
FetchContent_Declare(
  entt
  GIT_REPOSITORY https://github.com/skypjack/entt.git
  GIT_TAG        fef9211 # v3.11.1 
)
set(ENTT_INCLUDE_NATVIS ON )
FetchContent_MakeAvailable(entt)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        b796f7d # v1.13.0 
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(third_party)

find_package(glm CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/engine)

message(STATUS "Source directory: ${ENGINE_DIR}")
message(STATUS "Binary directory: ${PROJECT_BINARY_DIR}")
message(STATUS "BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "VulkanSDK: $ENV{VULKAN_SDK}")
message(STATUS "Vulkan Lib: ${Vulkan_LIBRARIES}")
message(STATUS "Vulkan Inc: ${Vulkan_INCLUDE_DIRS}")

function (BuildTool tool external_target external_includes)
  set(TOOL_DIR ${CMAKE_CURRENT_SOURCE_DIR})
  set(BIN_DIR ${CMAKE_BINARY_DIR}/bin/${tool})
  set(EXE_DIR ${BIN_DIR}/${CMAKE_BUILD_TYPE})
  file(GLOB TOOL_SOURCES "${TOOL_DIR}/tool/src/*.cpp")
  file(GLOB TOOL_INCLUDES "${TOOL_DIR}/tool/include/*.h")

  message(STATUS "building tool: ${tool}")
  message(STATUS "include: ${TOOL_INCLUDES}")
  message(STATUS "sources: ${TOOL_SOURCES}")

  message(STATUS "external target: ${external_target}")
  message(STATUS "external includes: ${external_includes}")

  if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
  else()                                                                          
    set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
  endif()

  file(GLOB_RECURSE GLSL_SOURCE_FILES 
    "shaders/*.frag"
    "shaders/*.vert")

  file(GLOB_RECURSE OBJ_SOURCE_FILES 
    "assets/*.obj"
    "assets/*.mtl")

  foreach(GLSL ${GLSL_SOURCE_FILES}) 
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${EXE_DIR}/shaders/${FILE_NAME}.spv")
    message(STATUS "HLSL->SPIRV: ${SPIRV}")
    add_custom_command(
      OUTPUT ${SPIRV}
      COMMAND ${CMAKE_COMMAND} -E make_directory "${EXE_DIR}/shaders/"
      COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
      DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
  endforeach(GLSL)

  add_custom_target(${tool}_Shaders DEPENDS ${SPIRV_BINARY_FILES})

  add_executable(${tool} "tool/main.cpp" ${TOOL_SOURCES} ${TOOL_INCLUDES})
  target_include_directories(${tool} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/tool/include")
  target_include_directories(${tool} PUBLIC ${ENGINE_DIR}/include)
  target_include_directories(${tool} PUBLIC ${Vulkan_INCLUDE_DIRS})
  target_include_directories(${tool} PUBLIC ${SDL2_INCLUDE_DIRS})
  target_include_directories(${tool} PUBLIC ${imgui_DIR} ${imgui_DIR}/backends)
  target_include_directories(${tool} PUBLIC ../third_party/tinyobjloader/tiny_obj_loader.h)
  target_link_libraries(${tool} PUBLIC engine_lib)

  if (EXISTS ${external_target})
    target_link_libraries(${tool} PUBLIC ${external_target})
  endif()

  if (EXISTS ${external_includes})
    target_include_directories(${tool} PUBLIC ${external_includes})
  endif()

  add_dependencies(${tool} ${tool}_Shaders)

  set_target_properties(${tool}
      PROPERTIES
      ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
      LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
      RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
  )

  foreach (OBJ_FILE ${OBJ_SOURCE_FILES})
    get_filename_component(FILE_NAME ${OBJ_FILE} NAME)
    add_custom_command(
          TARGET ${tool} POST_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory "${EXE_DIR}/shaders/"
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${OBJ_FILE}
                  ${EXE_DIR}/assets/${FILE_NAME})
  endforeach(OBJ_FILE)
endfunction()

add_subdirectory(engine)
add_subdirectory(vulkan_tutorial)
add_subdirectory(formation_sim)
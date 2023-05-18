project(engine)

message(STATUS "BUILDING ENGINE")

file(GLOB ENGINE_SOURCES 
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/world/*.cpp")
file(GLOB ENGINE_INCLUDES 
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/world/*.h")

message(STATUS "engine src: ${ENGINE_SOURCES}")
message(STATUS "engine inc: ${ENGINE_INCLUDES}")



add_library(engine_lib STATIC ${ENGINE_INCLUDES} ${ENGINE_SOURCES})


target_sources(engine_lib PRIVATE
  "${imgui_DIR}/imgui.h"
  "${imgui_DIR}/imgui.cpp"

  "${imgui_DIR}/imgui_demo.cpp"
  "${imgui_DIR}/imgui_draw.cpp"
  "${imgui_DIR}/imgui_widgets.cpp"
  "${imgui_DIR}/imgui_tables.cpp"

  "${imgui_DIR}/backends/imgui_impl_vulkan.cpp"
  "${imgui_DIR}/backends/imgui_impl_sdl2.cpp"
)

target_include_directories(engine_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(engine_lib PUBLIC ${Vulkan_INCLUDE_DIRS})
target_include_directories(engine_lib PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(engine_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/vma)
target_include_directories(engine_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/external/stb)
target_include_directories(engine_lib PUBLIC ${imgui_DIR} ${imgui_DIR}/backends)
target_include_directories(engine_lib PUBLIC ${box2d_INCLUDE})
target_include_directories(engine_lib PUBLIC ${ReactPhysics3D_INCLUDE})
target_link_libraries(engine_lib
    PUBLIC
      ${Vulkan_LIBRARIES} 
      sdl2
      tinyobjloader
      glm::glm
      Boost::boost
      spdlog::spdlog spdlog::spdlog_header_only
      vk-bootstrap::vk-bootstrap
      tinyobjloader
      EnTT::EnTT
      ${box2d_LIB}
      ${ReactPhysics3D_LIB}
  )
set_target_properties(engine_lib PROPERTIES LINKER_LANGUAGE CXX)

add_subdirectory(tst)


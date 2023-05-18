
Sketchbook
=====


A demo graphics engine written in C++ called SketchBook with Dear ImGUI support based on this [vulkan c++ guide](https://vkguide.dev/). It is designed to facilitate the development of Vulkan-based applications. The engine provides a templated Engine class that inherits from the Core::EngineVulkanBase class and includes various functionality for setting up scenes, handling cameras, rendering pipelines, materials, and meshes. The engine utilizes libraries such as glm, imgui, and spdlog for additional features.

The Engine class allows users to set up and manage scenes, cameras, and various rendering-related components. It provides functions for creating pipelines, pipeline layouts, materials, and meshes. The code also includes implementations for setting up ImGui for GUI functionality and initializes the Vulkan rendering context. The draw function handles the rendering process, including acquiring swapchain images, recording command buffers, and submitting them to the graphics queue for rendering.

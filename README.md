# Vulkan sample applications

<br>![Khronos Vulkan logo](https://raw.githubusercontent.com/AnselmoGPP/Vulkan_samples/master/files/Khronos-Vulkan-Logo_2.png)

This repository contains different Vulkan projects and the dependencies needed for building a Vulkan project.

<h3>Content of this repository:</h3>

- _**src:**_ Different projects that use Vulkan
  - **Vk_1:** The very basics. Triangle 
  - **Vk_2:** Buffers (Staging, Vertex, Index). Square. 
  - **Vk_3:** Descriptors (layout, buffer, pool, sets). Rotating 3D square. 
  - **Vk_4:** Texture mapping (image view and sampler). Rotating 3D textured square.
  - **Vk_5:** Depth buffering. Two rotating 3D texture squares at different heights.
  - **Vk_6:** OBJ models loaders. Loads a 3D scenario.
  - **Vk_7:** Mipmapping. In software generation.
  - **Vk_8:** Multisampling: Shader MSAA (MultiSampling AntiAliasing) & SS (Sample shading).
- _**extern:**_ Dependencies
- _**files:**_ Scripts and images
- _**textures:**_ Images used as textures in our Vulkan projects

<br>![Vulkan window](https://raw.githubusercontent.com/AnselmoGPP/Vulkan_samples/master/files/vulkan_window.png)

<h3>Dependencies:</h3>

- **GLFW** (Window system and inputs)
- **GLM** (Mathematics library)
- **stb_image** (Image loader)
- **tinyobjloader** (Load vertices and faces from an OBJ file)
- **Vulkan SDK** (Set of repositories useful for Vulkan development) (installed in platform-specfic directories)
  - Vulkan loader (Khronos)
  - Vulkan validation layer (Khronos)
  - Vulkan extension layer (Khronos)
  - Vulkan tools (Khronos)
  - Vulkan tools (LunarG)
  - gfxreconstruct (LunarG)
  - glslang (Shader compiler to SPIR-V) (Khronos)
  - shaderc (C++ API wrapper around glslang) (Google)
  - SPIRV-Tools (Khronos)
  - SPIRV-Cross (Khronos)
  - SPIRV-Reflect (Khronos)

<h3>Steps for building this project:</h3>

The following includes the basics for setting up this project. For more details about setting up Vulkan, check [Setting up Vulkan](https://sciencesoftcode.wordpress.com/2021/03/09/setting-up-vulkan/).

- <h4>Ubuntu:</h4>

  - Update your GPU's drivers
  - Get:
    1. Compiler that supports C++17
    2. Make
    3. CMake
  - Install Vulkan SDK
    1. [Download](https://vulkan.lunarg.com/sdk/home) tarball in `extern\`.
    2. Run script `./files/install_vulkansdk` (modify `pathVulkanSDK` variable if necessary)
  - Build project using the scripts:
    1. `sudo ./files/build_dependencies_ubuntu`
    2. `./files/build_project_ubuntu`

- <h4>Windows:</h4>

  - Update your GPU's drivers
  - Install Vulkan SDK
    1. [Download](https://vulkan.lunarg.com/sdk/home) installer wherever 
    2. Execute installer
  - Get:
    1. MVS
    2. CMake
  - Build project using the scripts:
    1. `build_dependencies_Win.bat`
    2. `build_project_Win.bat`
  - Compile project with MVS (Set as startup project & Release mode)

<h3>Links:</h3>

- [Setting up Vulkan](https://sciencesoftcode.wordpress.com/2021/03/09/setting-up-vulkan/)
- [Vulkan tutorials] (https://wordpress.com/post/sciencesoftcode.wordpress.com/1654)
- [Vulkan SDK (getting started)](https://vulkan.lunarg.com/doc/sdk/1.2.170.0/linux/getting_started.html)
- [Vulkan tutorial](https://vulkan-tutorial.com/)

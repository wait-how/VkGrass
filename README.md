# VkGrass
Grass Demo
## Installation
Compilation deps:
 - nproc
 - pkg-config
 - some recent version of clang++ and lld
 - vulkan headers:
   - vulkan-headers
   - vulkan-validation-layers (for debugging)
   - vulkan-tools (for the very useful vulkaninfo command)

Runtime deps (listed as arch packages):
 - a vulkan-capable driver (only intel and nvidia tested!)
 - assimp
 - glm
 - glfw

TODO:
0. create 3D models
1. Render hills
2. Render grass
  - get OIT working
  - Use "blended OIT" since alphas are zero and colors are similar!
3. Render skybox
4. Render sun
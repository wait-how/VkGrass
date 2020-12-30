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
1. Render grass
  - use tesselation shader for lod'ing things if required
  - get grass to move from wind
  - why do I have to flip UVs for grass?
2. Render skybox
3. Render sun

Credits:
Grass billboard texture: [Reiner “Tiles” Prokein](https://www.reinerstilesets.de/graphics/lizenz/)
Everything else: me
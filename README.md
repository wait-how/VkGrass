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
2. Render grass
  - render instances of grass all over the hills
  - use tesselation shader for lod'ing things if required
  - use alpha to get grass to overlap
  - get grass to move from wind
3. Render skybox
4. Render sun

Credits:
Grass billboard texture: [Reiner “Tiles” Prokein](https://www.reinerstilesets.de/graphics/lizenz/)
Everything else: me
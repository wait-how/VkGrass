# VkGrass
Grass Demo
## Installation
Build dependencies:
 - pkg-config
 - some recent version of clang++/lld (g++/ld should work, but is not tested)

Compilation dependencies:
 - glm
 - a vulkan driver, and the following pacman packages:
   - vulkan-headers (for compiling)
   - vulkan-validation-layers (for debugging)
   - vulkan-tools (for the very useful vulkaninfo command)

Runtime dependencies:
 - a vulkan-capable driver/card
 - assimp
 - glfw

TODO:
1. Render grass
  - Cull faraway objects on CPU
  - get grass to move from wind
  - why do I have to flip UVs for grass?
2. Render skybox
3. Render sun

Optimizations:
1. Make one large memory alloc and break off chunks of it for buffers

Credits:
 - Grass billboard texture: [Reiner “Tiles” Prokein](https://www.reinerstilesets.de/graphics/lizenz/)
 - Everything else: me
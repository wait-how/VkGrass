# VkGrass
A basic grass demo
![screenshot](VkGrass.jpg)
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

## Controls
 - `WSAD` to move
 - `Space` to go up
 - `Lshift` to go down
 - `R` to reset the camera

## TODO
0. Procedural terrain
 - vertex normal alg:
   - calculate face normals by n = (c - a) x (c - b)
   - for each vertex, average face normals that are adjacent to that vertex
1. Render grass
  - Cull faraway objects on CPU
  - get grass to move from wind
  - why do I have to flip UVs for grass?
2. Render skybox
3. Render sun

Bugs:
- camera broken with optimization?

Optimizations:
1. Make one large memory alloc and break off chunks of it for buffers

Credits:
 - Grass billboard texture: [Reiner “Tiles” Prokein](https://www.reinerstilesets.de/graphics/lizenz/)
 - Everything else: me
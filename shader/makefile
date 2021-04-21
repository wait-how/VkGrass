CC := glslangValidator

# stages named <name>.vert, <vert>.frag, etc. get
# converted to vert.spv, frag.spv, etc by glslang

SPV := terrain_v.spv terrain_f.spv grass_v.spv grass_f.spv skybox_v.spv skybox_f.spv
BUILD = $(CC) --target-env vulkan1.2 -t $^ -o $@

.PHONY: clean

all: $(SPV)

terrain_v.spv: terrain_v.vert
	$(BUILD)

terrain_f.spv: terrain_f.frag
	$(BUILD)

grass_v.spv: grass_v.vert
	$(BUILD)

grass_f.spv: grass_f.frag
	$(BUILD)

skybox_v.spv: skybox_v.vert
	$(BUILD)

skybox_f.spv: skybox_f.frag
	$(BUILD)

clean:
	@rm -f *.spv


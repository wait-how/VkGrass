#pragma once

#include "vformat.hpp"

#include <vector>
#include <string>

#include <assimp/scene.h>

namespace vload {

	class mesh {
	public:
		std::vector<vformat::vertex> verts;
		std::vector<uint32_t> indices; // for index buffer

		mesh(const std::vector<vformat::vertex>& inVertList, const std::vector<uint32_t>& inElemList) : verts(inVertList), indices(inElemList) { }
		mesh() : verts(), indices() { }
	};

	class vloader {
	public:
		vloader(std::string_view path, bool makeIndices);
		std::vector<mesh> meshList;
	private:
		void processNode(aiNode* node, const aiScene* scene);
		mesh processMesh(aiMesh* inMesh);
	};
}

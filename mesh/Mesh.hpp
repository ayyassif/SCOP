#ifndef MESH_HPP
#define MESH_HPP

#include "SubMesh.hpp"

#include <vector>

struct Mesh
{
	std::vector<SubMesh> subMeshes;
};

#endif

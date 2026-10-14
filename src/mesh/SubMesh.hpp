#ifndef SUBMESH_HPP
#define SUBMESH_HPP

#include "Vertex.hpp"

#include <string>
#include <vector>

struct SubMesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::string materialName;
};

#endif

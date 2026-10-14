#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include "Vertex.hpp"

struct Mesh
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

#endif

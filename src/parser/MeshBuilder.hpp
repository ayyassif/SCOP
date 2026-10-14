#ifndef MESHBUILDER_HPP
#define MESHBUILDER_HPP

#include "OBJTypes.hpp"
#include "../mesh/Mesh.hpp"

class MeshBuilder
{
public:
	Mesh build(const OBJData& data);
};

#endif

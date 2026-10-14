#ifndef OBJTYPES_HPP
#define OBJTYPES_HPP

#include "../math/Math.hpp"
#include <vector>
#include <string>

struct OBJIndex
{
    int position = -1;
    int texcoord = -1;
    int normal = -1;
};

struct OBJFace
{
    std::vector<OBJIndex> vertices;
    std::string materialName;
    int chartIndex = -1;
};

struct OBJData
{
    std::vector<Vec3> positions;
    std::vector<Vec2> texcoords;
    std::vector<Vec3> normals;
    std::vector<OBJFace> faces;
    std::string materialLibrary;
};

#endif
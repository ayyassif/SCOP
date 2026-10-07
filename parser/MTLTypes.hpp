#ifndef MTLTYPES_HPP
#define MTLTYPES_HPP

#include "../math/Math.hpp"
#include <string>
#include <unordered_map>

struct Material
{
    std::string name;
    float Ns = 0.0f;
    Vec3 Ka{0.0f, 0.0f, 0.0f};
    Vec3 Kd{0.0f, 0.0f, 0.0f};
    Vec3 Ks{0.0f, 0.0f, 0.0f};
    float Ni = 1.0f;
    float d = 1.0f;
    int illum = 0;
    std::string diffuseTexture;
};

struct MTLData
{
    std::unordered_map<std::string, Material> materials;
};

#endif

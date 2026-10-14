#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include "../math/Vec3.hpp"
#include <string>

struct Material
{
    std::string name;

    float Ns;
    Vec3 Ka;
    Vec3 Kd;
    Vec3 Ks;
    float Ni;
    float d;
    int illum;

    std::string diffuseTexture;
};

#endif

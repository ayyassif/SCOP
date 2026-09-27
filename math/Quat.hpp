#ifndef QUAT_HPP
#define QUAT_HPP

#include <cmath>

#include "Mat4.hpp"
#include "Vec3.hpp"

class Quat
{
public:
    float w;
    float x;
    float y;
    float z;

    Quat();
    Quat(float w, float x, float y, float z);

    static Quat identity();
    static Quat fromAxisAngle(const Vec3& axis, float angle);

    float length() const;
    Quat normalize() const;
    Quat conjugate() const;
    Quat inverse() const;

    Quat operator*(const Quat& other) const;
    Quat operator*(float scalar) const;

    Vec3 rotate(const Vec3& v) const;

    Mat4 toMat4() const;
};

#endif
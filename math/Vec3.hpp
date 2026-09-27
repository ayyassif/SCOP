#ifndef VEC3_HPP
#define VEC3_HPP

#include <cmath>

class Vec3
{
public:
    float x;
    float y;
    float z;

    float& operator[](int index);
    const float& operator[](int index) const;

    Vec3();
    Vec3(float value);
    Vec3(float x, float y, float z);

    static Vec3 normalize(Vec3 v);

    float length() const;
    Vec3 normalize() const;

    float dot(const Vec3& v) const;
    Vec3 cross(const Vec3& v) const;
    

    Vec3 operator+(const Vec3& v) const;
    Vec3 operator-(const Vec3& v) const;
    Vec3 operator*(float scalar) const;
    Vec3 operator/(float scalar) const;

    Vec3& operator+=(const Vec3& v);
    Vec3& operator-=(const Vec3& v);
    Vec3& operator*=(float scalar);
};

#endif
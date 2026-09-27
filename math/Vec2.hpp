#ifndef VEC2_HPP
#define VEC2_HPP

#include <cmath>

class Vec2
{
public:
    float x;
    float y;

    float& operator[](int index);
    const float& operator[](int index) const;

    Vec2();
    Vec2(float x, float y);

    float length() const;
    Vec2 normalize() const;

    Vec2 operator+(const Vec2& v) const;
    Vec2 operator-(const Vec2& v) const;
    Vec2 operator*(float scalar) const;
    Vec2 operator/(float scalar) const;

    float dot(const Vec2& v) const;
};

#endif
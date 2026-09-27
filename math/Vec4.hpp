#ifndef VEC4_HPP
#define VEC4_HPP

class Vec4
{
public:
    float x;
    float y;
    float z;
    float w;

    float& operator[](int index);
    const float& operator[](int index) const;

    Vec4();
    Vec4(float x, float y, float z, float w);

    Vec4 operator+(const Vec4& v) const;
    Vec4 operator-(const Vec4& v) const;
    Vec4 operator*(float scalar) const;
};

#endif
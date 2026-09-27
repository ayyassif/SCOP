#include "Vec4.hpp"

float& Vec4::operator[](int index)
{
    if (index == 0) return x;
    if (index == 1) return y;
    if (index == 2) return z;
    return w;
}

const float& Vec4::operator[](int index) const
{
    if (index == 0) return x;
    if (index == 1) return y;
    if (index == 2) return z;
    return w;
}

Vec4::Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}

Vec4::Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}

Vec4 Vec4::operator+(const Vec4& v) const
{
	return Vec4(x + v.x, y + v.y, z + v.z, w + v.w);
}

Vec4 Vec4::operator-(const Vec4& v) const
{
	return Vec4(x - v.x, y - v.y, z - v.z, w - v.w);
}

Vec4 Vec4::operator*(float scalar) const
{
	return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
}

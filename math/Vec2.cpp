#include "Vec2.hpp"

float& Vec2::operator[](int index)
{
    return (index == 0) ? x : y;
}

const float& Vec2::operator[](int index) const
{
    return (index == 0) ? x : y;
}

Vec2::Vec2() : x(0.0f), y(0.0f) {}

Vec2::Vec2(float x_, float y_) : x(x_), y(y_) {}

float Vec2::length() const
{
	return std::sqrt(x * x + y * y);
}

Vec2 Vec2::normalize() const
{
	const float len = length();
	if (len == 0.0f)
		return Vec2();
	return Vec2(x / len, y / len);
}

Vec2 Vec2::operator+(const Vec2 &v) const
{
	return Vec2(x + v.x, y + v.y);
}

Vec2 Vec2::operator-(const Vec2 &v) const
{
	return Vec2(x - v.x, y - v.y);
}

Vec2 Vec2::operator*(float scalar) const
{
	return Vec2(x * scalar, y * scalar);
}

Vec2 Vec2::operator/(float scalar) const
{
	return Vec2(x / scalar, y / scalar);
}

float Vec2::dot(const Vec2 &v) const
{
	return x * v.x + y * v.y;
}

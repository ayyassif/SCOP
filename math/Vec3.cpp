#include "Vec3.hpp"

Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}

Vec3::Vec3(float value) : x(value), y(value), z(value) {}

Vec3::Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

Vec3 Vec3::normalize(Vec3 v)
{
    const float len = v.length();
    if (len == 0.0f) {
        return Vec3();
    }
    return v / len;
}

float& Vec3::operator[](int index)
{
    if (index == 0) return x;
    if (index == 1) return y;
    return z;
}

const float& Vec3::operator[](int index) const
{
    if (index == 0) return x;
    if (index == 1) return y;
    return z;
}

Vec3 Vec3::operator+(const Vec3& rhs) const
{
	return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
}

Vec3 Vec3::operator-(const Vec3& rhs) const
{
	return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
}

Vec3 Vec3::operator*(float scalar) const
{
	return Vec3(x * scalar, y * scalar, z * scalar);
}

Vec3 Vec3::operator/(float scalar) const
{
	return Vec3(x / scalar, y / scalar, z / scalar);
}

Vec3& Vec3::operator+=(const Vec3& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

Vec3& Vec3::operator-=(const Vec3& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	return *this;
}

Vec3& Vec3::operator*=(float scalar)
{
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

float Vec3::dot(const Vec3& rhs) const
{
	return x * rhs.x + y * rhs.y + z * rhs.z;
}

Vec3 Vec3::cross(const Vec3& rhs) const
{
	return Vec3(
		y * rhs.z - z * rhs.y,
		z * rhs.x - x * rhs.z,
		x * rhs.y - y * rhs.x
	);
}

float Vec3::length() const
{
	return std::sqrt(dot(*this));
}

Vec3 Vec3::normalize() const
{
	const float len = length();
	if (len == 0.0f) {
		return Vec3();
	}
	return *this / len;
}


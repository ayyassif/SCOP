#include "Quat.hpp"

Quat::Quat() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}

Quat::Quat(float w_, float x_, float y_, float z_) : w(w_), x(x_), y(y_), z(z_) {}

Quat Quat::identity()
{
	return Quat();
}

Quat Quat::fromAxisAngle(const Vec3& axis, float angle)
{
	const Vec3 n = axis.normalize();
	const float halfAngle = angle * 0.5f;
	const float sinHalf = std::sin(halfAngle);

	return Quat(
		std::cos(halfAngle),
		n.x * sinHalf,
		n.y * sinHalf,
		n.z * sinHalf
	);
}

float Quat::length() const
{
	return std::sqrt(w * w + x * x + y * y + z * z);
}

Quat Quat::normalize() const
{
	const float len = length();
	if (len == 0.0f) {
		return Quat::identity();
	}
	return Quat(w / len, x / len, y / len, z / len);
}

Quat Quat::conjugate() const
{
	return Quat(w, -x, -y, -z);
}

Quat Quat::inverse() const
{
	const float lenSq = w * w + x * x + y * y + z * z;
	if (lenSq == 0.0f) {
		return Quat::identity();
	}
	return conjugate() * (1.0f / lenSq);
}

Quat Quat::operator*(const Quat& other) const
{
	return Quat(
		w * other.w - x * other.x - y * other.y - z * other.z,
		w * other.x + x * other.w + y * other.z - z * other.y,
		w * other.y - x * other.z + y * other.w + z * other.x,
		w * other.z + x * other.y - y * other.x + z * other.w
	);
}

Quat Quat::operator*(float scalar) const
{
	return Quat(w * scalar, x * scalar, y * scalar, z * scalar);
}

Vec3 Quat::rotate(const Vec3& v) const
{
	const Quat q = normalize();
	const Quat pure(0.0f, v.x, v.y, v.z);
	const Quat rotated = q * pure * q.conjugate();
	return Vec3(rotated.x, rotated.y, rotated.z);
}

Mat4 Quat::toMat4() const
{
	const Quat q = normalize();

	const float xx = q.x * q.x;
	const float yy = q.y * q.y;
	const float zz = q.z * q.z;
	const float xy = q.x * q.y;
	const float xz = q.x * q.z;
	const float yz = q.y * q.z;
	const float wx = q.w * q.x;
	const float wy = q.w * q.y;
	const float wz = q.w * q.z;

	Mat4 result = Mat4::identity();
	result.m[0][0] = 1.0f - 2.0f * (yy + zz);
	result.m[0][1] = 2.0f * (xy - wz);
	result.m[0][2] = 2.0f * (xz + wy);

	result.m[1][0] = 2.0f * (xy + wz);
	result.m[1][1] = 1.0f - 2.0f * (xx + zz);
	result.m[1][2] = 2.0f * (yz - wx);

	result.m[2][0] = 2.0f * (xz - wy);
	result.m[2][1] = 2.0f * (yz + wx);
	result.m[2][2] = 1.0f - 2.0f * (xx + yy);

	return result;
}

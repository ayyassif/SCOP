#ifndef VEC3_HPP
#define VEC3_HPP

#include <cmath>

template <typename T = float>
class Vec3T
{
public:
    T x;
    T y;
    T z;

    T& operator[](int index)
    {
        if (index == 0) return x;
        if (index == 1) return y;
        return z;
    }

    const T& operator[](int index) const
    {
        if (index == 0) return x;
        if (index == 1) return y;
        return z;
    }

    Vec3T() : x(0), y(0), z(0) {}
    Vec3T(T value) : x(value), y(value), z(value) {}
    Vec3T(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}

    static Vec3T normalized(Vec3T v, const Vec3T& fallback = Vec3T())
    {
        return v.normalize(fallback);
    }

    T length() const
    {
        return static_cast<T>(std::sqrt(dot(*this)));
    }

    Vec3T normalize(const Vec3T& fallback = Vec3T()) const
    {
        const T len = length();
        if (len == T(0)) {
            return fallback;
        }
        return *this / len;
    }

    T dot(const Vec3T& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    Vec3T cross(const Vec3T& v) const
    {
        return Vec3T(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    Vec3T operator+(const Vec3T& v) const
    {
        return Vec3T(x + v.x, y + v.y, z + v.z);
    }

    Vec3T operator-(const Vec3T& v) const
    {
        return Vec3T(x - v.x, y - v.y, z - v.z);
    }

    Vec3T operator*(T scalar) const
    {
        return Vec3T(x * scalar, y * scalar, z * scalar);
    }

    Vec3T operator/(T scalar) const
    {
        return Vec3T(x / scalar, y / scalar, z / scalar);
    }

    Vec3T& operator+=(const Vec3T& v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vec3T& operator-=(const Vec3T& v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    Vec3T& operator*=(T scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
};

using Vec3 = Vec3T<float>;

#endif
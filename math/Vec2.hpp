#ifndef VEC2_HPP
#define VEC2_HPP

#include <cmath>

template <typename T = float>
class Vec2T
{
public:
    T x;
    T y;

    T& operator[](int index)
    {
        return (index == 0) ? x : y;
    }

    const T& operator[](int index) const
    {
        return (index == 0) ? x : y;
    }

    Vec2T() : x(0), y(0) {}
    Vec2T(T x_, T y_) : x(x_), y(y_) {}

    T length() const
    {
        return static_cast<T>(std::sqrt(x * x + y * y));
    }

    Vec2T normalize() const
    {
        const T len = length();
        if (len == T(0))
            return Vec2T();
        return Vec2T(x / len, y / len);
    }

    Vec2T operator+(const Vec2T& v) const
    {
        return Vec2T(x + v.x, y + v.y);
    }

    Vec2T operator-(const Vec2T& v) const
    {
        return Vec2T(x - v.x, y - v.y);
    }

    Vec2T operator*(T scalar) const
    {
        return Vec2T(x * scalar, y * scalar);
    }

    Vec2T operator/(T scalar) const
    {
        return Vec2T(x / scalar, y / scalar);
    }

    T dot(const Vec2T& v) const
    {
        return x * v.x + y * v.y;
    }
};

using Vec2 = Vec2T<float>;

#endif
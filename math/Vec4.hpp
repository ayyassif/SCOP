#ifndef VEC4_HPP
#define VEC4_HPP

template <typename T = float>
class Vec4T
{
public:
    T x;
    T y;
    T z;
    T w;

    T& operator[](int index)
    {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;
        return w;
    }

    const T& operator[](int index) const
    {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;
        return w;
    }

    Vec4T() : x(0), y(0), z(0), w(0) {}
    Vec4T(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}

    Vec4T operator+(const Vec4T& v) const
    {
        return Vec4T(x + v.x, y + v.y, z + v.z, w + v.w);
    }

    Vec4T operator-(const Vec4T& v) const
    {
        return Vec4T(x - v.x, y - v.y, z - v.z, w - v.w);
    }

    Vec4T operator*(T scalar) const
    {
        return Vec4T(x * scalar, y * scalar, z * scalar, w * scalar);
    }
};

using Vec4 = Vec4T<float>;

#endif
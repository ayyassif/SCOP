#ifndef MAT4_HPP
#define MAT4_HPP

#include <cmath>

#include "Vec3.hpp"
#include "Vec4.hpp"

class Mat4
{
private:
    float m[4][4];

public:
    float* operator[](int index);
    const float* operator[](int index) const;

    Mat4();
    Mat4(float diagonal);

    static Mat4 identity();

    static Mat4 translate(const Vec3& position);
    static Mat4 scale(const Vec3& scale);
    static Mat4 rotate(float angle, const Vec3& axis);
    static Mat4 rotationX(float angle);
    static Mat4 rotationY(float angle);
    static Mat4 rotationZ(float angle);
    static Mat4 model(
        const Vec3& trans = Vec3(0.0f),
        float angle = 0.0f,
        const Vec3& axis = Vec3(0.0f, 0.0f, 1.0f),
        const Vec3& scale = Vec3(1.0f)
    );

    static Mat4 perspective(
        float fov,
        float aspect,
        float nearPlane,
        float farPlane
    );

    // static Mat4 orthographic(
    //     float left,
    //     float right,
    //     float bottom,
    //     float top,
    //     float nearPlane,
    //     float farPlane
    // );

    static Mat4 lookAt(
        const Vec3& position,
        const Vec3& target,
        const Vec3& worldUp
    );

    // Mat4 transpose() const;
    // Mat4 inverse() const;

    const float* toColumnMajor() const;

    Mat4 operator*(const Mat4& other) const;
    Vec4 operator*(const Vec4& v) const;
};

#endif
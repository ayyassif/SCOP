#ifndef MATH_HPP
#define MATH_HPP

#include <cmath>
#include <GLFW/glfw3.h>
#include "Mat4.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"
#include "Vec4.hpp"

namespace Math
{
    float radians(float degrees);
    float getDeltaTime( void );
}

#endif
#include "Math.hpp"

float Math::radians(float degrees)
{
    return degrees * (3.14159265358979323846f / 180.0f);
}

float Math::getDeltaTime( void )
{
    static float lastFrame = glfwGetTime();
    float currentFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    return (deltaTime);
}
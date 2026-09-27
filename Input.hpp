#ifndef INPUT_HPP
#define INPUT_HPP

#include "math/Vec2.hpp"
#include <GLFW/glfw3.h>

class Input
{
private:
    bool firstMouse;
    Vec2 lastMouse;

public:
    GLFWwindow *window;

    Input(GLFWwindow *window);

    bool getKeyPressed(int key) const;
    bool getKeyReleased(int key) const;

    Vec2 calculateMouseDelta(float xpos, float ypos);
};



Input::Input(GLFWwindow *window)
    : firstMouse(true), lastMouse(), window(window)
{
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lastMouse.x = (float)width / 2;
    lastMouse.y = (float)height / 2;
}

bool Input::getKeyPressed(int key) const
{
    return (glfwGetKey(window, key) == GLFW_PRESS);
}

bool Input::getKeyReleased(int key) const
{
    return (glfwGetKey(window, key) == GLFW_RELEASE);
}

Vec2 Input::calculateMouseDelta(float xpos, float ypos)
{
    Vec2 offset;
    offset.x = xpos - lastMouse.x;
    offset.y = lastMouse.y - ypos;
    lastMouse.x = xpos;
    lastMouse.y = ypos;

    if (firstMouse)
    {
        lastMouse.x = xpos;
        lastMouse.y = ypos;
        firstMouse = false;
    }

    return offset;
}

#endif
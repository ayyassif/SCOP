#include "Input.hpp"

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

void Input::resetMouseState()
{
    firstMouse = true;

    int width, height;
    glfwGetWindowSize(window, &width, &height);
    lastMouse.x = static_cast<float>(width) / 2.0f;
    lastMouse.y = static_cast<float>(height) / 2.0f;
}

Vec2 Input::calculateMouseDelta(float xpos, float ypos)
{
    if (firstMouse)
    {
        lastMouse.x = xpos;
        lastMouse.y = ypos;
        firstMouse = false;
        return Vec2();
    }

    Vec2 offset;
    offset.x = xpos - lastMouse.x;
    offset.y = lastMouse.y - ypos;
    lastMouse.x = xpos;
    lastMouse.y = ypos;

    return offset;
}
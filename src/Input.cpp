#include "Input.hpp"

Input::Input()
    : firstMouse(true),
    lastMouse(),
    window(nullptr),
    tabPressedLastFrame(false),
    mouseCaptured(false)
{}

Input::Input(GLFWwindow *window)
    : firstMouse(true),
    lastMouse(),
    window(window),
    tabPressedLastFrame(false),
    mouseCaptured(true)
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

void Input::processTabExit(const int &width, const int &height)
{
    if (getKeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, true);
        DEBUG::INPUT << "Exit requested" << std::endl;
    }

    const bool tabHeld = getKeyPressed(GLFW_KEY_TAB);
    if (tabHeld && !tabPressedLastFrame)
    {
        mouseCaptured = !mouseCaptured;
        if (mouseCaptured)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            resetMouseState();
        }
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        DEBUG::INPUT << "Mouse capture " << (mouseCaptured ? "enabled" : "disabled")
                 << std::endl;
    }
    tabPressedLastFrame = tabHeld;
}

bool Input::isMouseCaptured() const
{
    return mouseCaptured;
}
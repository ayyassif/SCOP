#ifndef INPUT_HPP
#define INPUT_HPP

#include "math/Vec2.hpp"
#include "Debugger.hpp"
#include <GLFW/glfw3.h>

class Input
{
private:
    bool firstMouse;
    Vec2 lastMouse;
    bool tabPressedLastFrame;
    bool mouseCaptured;
public:
    GLFWwindow *window;

    Input();
    Input(GLFWwindow *window);

    bool getKeyPressed(int key) const;
    bool getKeyReleased(int key) const;
    bool isMouseCaptured() const;
    void resetMouseState();

    Vec2 calculateMouseDelta(float xpos, float ypos);
    void processTabExit(const int &width, const int &height);
};

#endif
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
    void resetMouseState();

    Vec2 calculateMouseDelta(float xpos, float ypos);
};

#endif
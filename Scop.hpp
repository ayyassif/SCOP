#ifndef SCOP_HPP
#define SCOP_HPP

#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include <vector>

#include "ppm.hpp"
#include "Shader.hpp"
#include "math/Math.hpp"
#include "Camera.hpp"
#include "Input.hpp"

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

extern Camera cam;
extern GLFWwindow *window;
extern Input input;

GLFWwindow  *initiate();
std::string readFile(const char* filename);
void        framebuffer_size_callback(GLFWwindow* window, int width, int height);
void        processInput(GLFWwindow *window, Input &input, Camera &cam);
void        mouse_callback(GLFWwindow* window, double xpos, double ypos);
void        scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


class Scop
{
private:
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    unsigned int texture;
    
    void setupTexture(const PPM &image);
    void setupVertexBuffers(
        const std::vector<float>& vertices,
        const std::vector<float>& indices
    );

public:
    Scop(
        const std::vector<float>& vertices,
        const std::vector<float>& indices,
        const PPM &image
    );

    void loop();
};

#endif
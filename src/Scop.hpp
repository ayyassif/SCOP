#ifndef SCOP_HPP
#define SCOP_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <array>
#include <cstddef>
#include <vector>

#include "Debugger.hpp"
#include "ppm.hpp"
#include "Shader.hpp"
#include "math/Math.hpp"
#include "mesh/Vertex.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "mesh/Mesh.hpp"

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

extern Camera cam;
extern GLFWwindow *window;
extern Input input;

GLFWwindow  *initiate();
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
    std::size_t indexCount;
    
    void setupTexture(const PPM &image);
    void setupVertexBuffers(const Mesh& mesh);

public:
    Scop(
        const Mesh& mesh,
        const PPM&  image
    );

    ~Scop();

    Scop(const Scop&) = delete;
    Scop& operator=(const Scop&) = delete;

    void loop();
};

#endif
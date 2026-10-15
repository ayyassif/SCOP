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

GLFWwindow  *initiate();
void        framebuffer_size_callback(GLFWwindow* window, int width, int height);
void        mouse_callback(GLFWwindow* window, double xpos, double ypos);
void        scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// (Vec3(0.0f, 0.0f,  3.0f), Vec3(0.0f, 1.0f,  0.0f))
class Scop
{
private:
    GLFWwindow* window = nullptr;
    Camera      cam;
    Input       input;

    unsigned int VBO = 0;
    unsigned int VAO = 0;
    unsigned int EBO = 0;
    unsigned int texture = 0;
    std::size_t indexCount = 0;
    
    void setupTexture(const PPM &image);
    void setupVertexBuffers(const Mesh& mesh);
    void processInput();

public:
    Scop(const Mesh& mesh, const PPM& image);

    ~Scop();

    Scop(const Scop&) = delete;
    Scop& operator=(const Scop&) = delete;

    void loop();
};

#endif
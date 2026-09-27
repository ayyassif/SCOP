#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <array>
#include "Shader.hpp"
#include "math/Math.hpp"
#include "Camera.hpp"
#include "Input.hpp"

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

GLFWwindow *initiate();

Camera cam(Vec3(0.0f, 0.0f,  3.0f), Vec3(0.0f, 1.0f,  0.0f));
GLFWwindow *window = initiate();
Input  input(window);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow *initiate()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create a window with glfw
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "I LOVE WISH", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    
    // load opengl functions with glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return (window);
}


std::string readFile(const char* filename)
{
    std::ifstream file(filename);

    return std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
}



void processInput(GLFWwindow *window)
{
    const float deltaTime = Math::getDeltaTime();

    if (input.getKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

    if (input.getKeyPressed(GLFW_KEY_LEFT_CONTROL))
        cam.ProcessKeyboard(SPRINT_ON, deltaTime);
    else if (input.getKeyReleased(GLFW_KEY_LEFT_CONTROL))
        cam.ProcessKeyboard(SPRINT_OFF, deltaTime);

    constexpr std::array<std::pair<int, Camera_Movement>, 6> movementKeys = {{
        {GLFW_KEY_W, FORWARD},
        {GLFW_KEY_S, BACKWARD},
        {GLFW_KEY_D, RIGHT},
        {GLFW_KEY_A, LEFT},
        {GLFW_KEY_SPACE, UP},
        {GLFW_KEY_LEFT_SHIFT, DOWN}
    }};

    for (const auto& [key, movement] : movementKeys)
    {
        if (input.getKeyPressed(key))
            cam.ProcessKeyboard(movement, deltaTime);
    }
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    cam.ProcessMouseMovement(
        input.calculateMouseDelta(
            (float)xpos,
            (float)ypos
        )
    );
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cam.ProcessMouseScroll((float)yoffset);
}



int main()
{
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    

    ///////////////////////////////////////////////////
    Shader ourShader("shaders/vertex.glsl", "shaders/fragment.glsl");
    float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };  
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    

    // textures:
    unsigned int cTexture;
    glGenTextures(1, &cTexture); 
    glBindTexture(GL_TEXTURE_2D, cTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true); 

    int width, height, nrChannels;
    unsigned char *data = stbi_load("textures/container.jpg", &width, &height, &nrChannels, 0); 
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load cTexture" << std::endl;
        return 1;
    }
    stbi_image_free(data);


    unsigned int wTexture;
    glGenTextures(1, &wTexture);
    glBindTexture(GL_TEXTURE_2D, wTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    data = stbi_load("textures/awesomeface.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load wTexture" << std::endl;
        return 1;
    }
    stbi_image_free(data);

    // transformation

    
    ///////////////////////////////////////////////////
    
    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    // ourShader.setMat4("mvp", projection * modelView);
    
    glEnable(GL_DEPTH_TEST);  
    float slider = 0.05;
    float fslider = 0.1f;

    Vec3 cubePositions[] = {
        Vec3( 0.0f,  0.0f,  0.0f),
        Vec3( 2.0f,  5.0f, -15.0f),
        Vec3(-1.5f, -2.2f, -2.5f),
        Vec3(-3.8f, -2.0f, -12.3f),
        Vec3 (2.4f, -0.4f, -3.5f),
        Vec3(-1.7f,  3.0f, -7.5f),
        Vec3( 1.3f, -2.0f, -2.5f),
        Vec3( 1.5f,  2.0f, -2.5f),
        Vec3( 1.5f,  0.2f, -1.5f),
        Vec3(-1.3f,  1.0f, -1.5f)
    };


    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback); 

    while(!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // rendering commands
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, cTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, wTexture);
        ourShader.use();
       
        Mat4 projection = Mat4::perspective(Math::radians(cam.Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f);
        
        glBindVertexArray(VAO);

        for (unsigned int i = 0; i < 10; i++)
        {
            float angle = i * 20.0f;
            Mat4 model = Mat4::model(cubePositions[i], Math::radians(angle), Vec3(1.0f, 0.3f, 0.5f));

            ourShader.setMat4("mvp", projection
                                    * cam.GetViewMatrix()
                                    * model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // check events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}


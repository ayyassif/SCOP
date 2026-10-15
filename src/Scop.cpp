#include "Scop.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

Camera *camPtr = nullptr;
Input *inputPtr = nullptr;

namespace
{
    int gFramebufferWidth = static_cast<int>(SCR_WIDTH);
    int gFramebufferHeight = static_cast<int>(SCR_HEIGHT);

    enum class RenderMode
    {
        Color = 0,
        Texture = 1,
        Grey = 2,
        ChartColors = 3
    };

    constexpr const char* renderNames[] =
    {
        "Color",
        "Texture",
        "Shades of Grey",
        "Chart Colors"
    };

    RenderMode toggleRenderMode(RenderMode mode, bool textureModes)
    {
        if (textureModes)
            return mode == RenderMode::Color
                ? RenderMode::Texture
                : RenderMode::Color;

        return mode == RenderMode::Grey
            ? RenderMode::ChartColors
            : RenderMode::Grey;
    }
}

 // <------------------------------------------------------------------------------------->
 // <-----------------------------------class functions----------------------------------->
 // <------------------------------------------------------------------------------------->

Scop::Scop(const Mesh& mesh, const PPM& image)
{
    if (mesh.vertices.empty() || mesh.indices.empty())
        throw std::invalid_argument("Mesh must contain vertices and indices");

    for (unsigned int index : mesh.indices)
    {
        if (index >= mesh.vertices.size())
            throw std::invalid_argument("Mesh contains an out-of-range index");
    }

    if (mesh.indices.size() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()))
        throw std::length_error("Mesh contains too many indices to render");

    window = initiate();
    if (window == nullptr)
        throw std::runtime_error("Window initialization failed");
    input = Input(window);
    camPtr = &cam;
    inputPtr = &input;
    indexCount = mesh.indices.size();
    DEBUG::INFO << "Creating render resources" << std::endl;
    cam.frameCameraOnVertices(mesh.vertices);
    setupVertexBuffers(mesh);
    setupTexture(image);
    DEBUG::SUCCESS << "Render resources ready" << std::endl;
}

Scop::~Scop()
{
    if (window != nullptr)
    {
        if (texture != 0)
            glDeleteTextures(1, &texture);
        if (VBO != 0)
            glDeleteBuffers(1, &VBO);
        if (EBO != 0)
            glDeleteBuffers(1, &EBO);
        if (VAO != 0)
            glDeleteVertexArrays(1, &VAO);

        glfwDestroyWindow(window);
        window = nullptr;
        glfwTerminate();
    }

    if (camPtr == &cam)
        camPtr = nullptr;
    if (inputPtr == &input)
        inputPtr = nullptr;
    DEBUG::SUCCESS << "Application closed normally" << std::endl;
}

void Scop::setupTexture(const PPM &image)
{
    glGenTextures(1, &texture); 
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        image.getWidth(),
        image.getHeight(),
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        image.getPixels().data()
    );

    glGenerateMipmap(GL_TEXTURE_2D);
    DEBUG::SUCCESS << "Texture uploaded to GPU" << std::endl;
}

void Scop::setupVertexBuffers(const Mesh& mesh)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        mesh.vertices.size() * sizeof(Vertex),
        mesh.vertices.data(), GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        mesh.indices.size() * sizeof(unsigned int),
        mesh.indices.data(), GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, position) + offsetof(Vec3, x)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, texcoord) + offsetof(Vec2, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex),
                           reinterpret_cast<void*>(offsetof(Vertex, chartIndex)));
    glEnableVertexAttribArray(2);
    DEBUG::SUCCESS << "Vertex buffers configured" << std::endl;
}

void Scop::loop()
{
    DEBUG::INFO << "Starting render loop" << std::endl;
    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    shader.use();
    shader.setInt("Texture", 0);
    RenderMode renderMode = RenderMode::Color;
    bool mKeyPressedLastFrame = false;
    bool nKeyPressedLastFrame = false;
    while(!glfwWindowShouldClose(window))
    {
        // input
        processInput();

        const bool mKeyPressed = input.getKeyPressed(GLFW_KEY_M);
        const bool nKeyPressed = input.getKeyPressed(GLFW_KEY_N);
        if (mKeyPressed && !mKeyPressedLastFrame)
        {
            renderMode = toggleRenderMode(renderMode, true);
            Debugger::renderModeChanged(renderNames[static_cast<int>(renderMode)]);
        }
        else if (nKeyPressed && !nKeyPressedLastFrame)
        {
            renderMode = toggleRenderMode(renderMode, false);
            Debugger::renderModeChanged(renderNames[static_cast<int>(renderMode)]);
        }

        mKeyPressedLastFrame = mKeyPressed;
        nKeyPressedLastFrame = nKeyPressed;

        glClearColor(0.12f, 0.15f, 0.19f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setInt("renderMode", static_cast<int>(renderMode));

        constexpr float nearPlane = 0.1f;
        constexpr float farPlane = 1000.0f;
        Mat4 projection = Mat4::perspective(
            Math::radians(cam.Zoom),
            static_cast<float>(std::max(1, gFramebufferWidth)) /
                static_cast<float>(std::max(1, gFramebufferHeight)),
            nearPlane,
            farPlane);

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        const Mat4 model = Mat4::model(Vec3(0.0f), 0.0f, Vec3(1.0f), Vec3(1.0f));

        shader.setMat4("mvp", projection
                                * cam.GetViewMatrix()
                                * model);

        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    DEBUG::INFO << "Render loop stopped" << std::endl;
}



 // <-------------------------------------------------------------------------------------->
 // <-----------------------------------helper functions----------------------------------->
 // <-------------------------------------------------------------------------------------->


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    gFramebufferWidth = width;
    gFramebufferHeight = height;
    glViewport(0, 0, width, height);
}

GLFWwindow *initiate()
{
    DEBUG::INFO << "Initializing GLFW" << std::endl;
    if (!glfwInit())
    {
        DEBUG::ERROR << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* createdWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SCOP", nullptr, nullptr);
    if (createdWindow == nullptr)
    {
        DEBUG::ERROR << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(createdWindow);
    
    // load opengl functions with glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        DEBUG::ERROR << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(createdWindow);
        glfwTerminate();
        return nullptr;
    }
    gFramebufferWidth = SCR_WIDTH;
    gFramebufferHeight = SCR_HEIGHT;
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(createdWindow, framebuffer_size_callback);

    glfwSetInputMode(createdWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(createdWindow, mouse_callback);
    glfwSetScrollCallback(createdWindow, scroll_callback);
    glEnable(GL_DEPTH_TEST);
    DEBUG::SUCCESS << "OpenGL context ready" << std::endl;
    return createdWindow;
}

void Scop::processInput()
{
    const float deltaTime = Math::getDeltaTime();

    input.processTabExit(
        gFramebufferWidth,
        gFramebufferHeight
    );

    if (cam.cameraSwitch(input.getKeyPressed(GLFW_KEY_C)))
        input.resetMouseState();

    if (!input.isMouseCaptured())
        return;

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

void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (camPtr == nullptr || inputPtr == nullptr || !inputPtr->isMouseCaptured())
        return;

    camPtr->ProcessMouseMovement(
        inputPtr->calculateMouseDelta(
            (float)xpos,
            (float)ypos
        )
    );
}

void scroll_callback(GLFWwindow*, double, double yoffset)
{
    if (camPtr == nullptr)
        return;

    camPtr->ProcessMouseScroll((float)yoffset);
}
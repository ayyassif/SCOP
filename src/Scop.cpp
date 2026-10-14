#include "Scop.hpp"

#include <algorithm>
#include <limits>

Camera cam(Vec3(0.0f, 0.0f,  3.0f), Vec3(0.0f, 1.0f,  0.0f));
GLFWwindow *window;
Input input;

namespace
{
    bool gMouseCaptured = true;
    bool gTabPressedLastFrame = false;
    bool gCameraModePressedLastFrame = false;
    int gFramebufferWidth = static_cast<int>(SCR_WIDTH);
    int gFramebufferHeight = static_cast<int>(SCR_HEIGHT);

    enum class RenderMode
    {
        Color = 0,
        Texture = 1,
        Grey = 2,
        ChartColors = 3
    };

    const std::string renderNames[] =
    {
        "Color",
        "Texture",
        "Shades of Grey",
        "Chart Colors"
    };

    void frameCameraOnVertices(const std::vector<Vertex>& vertices)
    {
        if (vertices.empty())
            return;

        Vec3 minimum(
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max(),
            std::numeric_limits<float>::max());
        Vec3 maximum(
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest(),
            std::numeric_limits<float>::lowest());

        for (const Vertex& vertex : vertices)
        {
            minimum.x = std::min(minimum.x, vertex.position.x);
            minimum.y = std::min(minimum.y, vertex.position.y);
            minimum.z = std::min(minimum.z, vertex.position.z);
            maximum.x = std::max(maximum.x, vertex.position.x);
            maximum.y = std::max(maximum.y, vertex.position.y);
            maximum.z = std::max(maximum.z, vertex.position.z);
        }

        const Vec3 center = (minimum + maximum) * 0.5f;
        const float radius = (maximum - minimum).length() * 0.5f;
        cam.FrameOn(center, radius);
    }
}

 // <------------------------------------------------------------------------------------->
 // <-----------------------------------class functions----------------------------------->
 // <------------------------------------------------------------------------------------->

Scop::Scop(
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices,
    const PPM &image
)
{
    indexCount = indices.size();
    DEBUG::INFO << "Creating render resources" << std::endl;
    frameCameraOnVertices(vertices);
    setupVertexBuffers(vertices, indices);
    setupTexture(image);
    DEBUG::SUCCESS << "Render resources ready" << std::endl;
}

Scop::~Scop()
{
    glDeleteTextures(1, &texture);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Scop::setupTexture(const PPM &image)
{
    int width;
    int height;
    int nrChannels;

    glGenTextures(1, &texture); 
    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // will change this later for changing textures when needed
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //  <---->
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

void Scop::setupVertexBuffers(
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices
)
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(), GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(), GL_STATIC_DRAW
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
        processInput(window, input, cam);

        const bool mKeyPressed = input.getKeyPressed(GLFW_KEY_M);
        const bool nKeyPressed = input.getKeyPressed(GLFW_KEY_N);
        if (mKeyPressed && !mKeyPressedLastFrame)
        {
            if (renderMode == RenderMode::Color)
                renderMode = RenderMode::Texture;
            else
                renderMode = RenderMode::Color;
            Debugger::renderModeChanged(renderNames[static_cast<int>(renderMode)]);
        }
        else if (nKeyPressed && !nKeyPressedLastFrame)
        {
            if (renderMode == RenderMode::Grey)
                renderMode = RenderMode::ChartColors;
            else
                renderMode = RenderMode::Grey;
            Debugger::renderModeChanged(renderNames[static_cast<int>(renderMode)]);
        }

        mKeyPressedLastFrame = mKeyPressed;
        nKeyPressedLastFrame = nKeyPressed;

        // rendering commands
        glClearColor(0.12f, 0.15f, 0.19f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setInt("renderMode", static_cast<int>(renderMode));

        constexpr float nearPlane = 0.1f;
        constexpr float farPlane = 1000.0f;
        Mat4 projection = Mat4::perspective(
            Math::radians(cam.Zoom),
            static_cast<float>(gFramebufferWidth) /
                static_cast<float>(std::max(1, gFramebufferHeight)),
            nearPlane,
            farPlane);

        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        Mat4 model = Mat4::model(Vec3(0.0f), Math::radians(0), Vec3(1.0f), Vec3(1.0f));

        shader.setMat4("mvp", projection
                                * cam.GetViewMatrix()
                                * model);

        // glDrawArrays(GL_TRIANGLES, 0, 36);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // check events and swap buffers
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

    // create a window with glfw
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SCOP", NULL, NULL);
    if (window == NULL)
    {
        DEBUG::ERROR << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    
    // load opengl functions with glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        DEBUG::ERROR << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }
    gFramebufferWidth = SCR_WIDTH;
    gFramebufferHeight = SCR_HEIGHT;
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback); 
    glEnable(GL_DEPTH_TEST);
    DEBUG::SUCCESS << "OpenGL context ready" << std::endl;
    return (window);
}

void processInput(GLFWwindow *window, Input &input, Camera &cam)
{
    const float deltaTime = Math::getDeltaTime();

    if (input.getKeyPressed(GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, true);
        DEBUG::INPUT << "Exit requested" << std::endl;
    }

    const bool tabHeld = input.getKeyPressed(GLFW_KEY_TAB);
    if (tabHeld && !gTabPressedLastFrame)
    {
        gMouseCaptured = !gMouseCaptured;
        if (gMouseCaptured)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetCursorPos(window, SCR_WIDTH / 2.0, SCR_HEIGHT / 2.0);
            input.resetMouseState();
        }
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        DEBUG::INPUT << "Mouse capture " << (gMouseCaptured ? "enabled" : "disabled")
                 << std::endl;
    }
    gTabPressedLastFrame = tabHeld;

    const bool cameraModePressed = input.getKeyPressed(GLFW_KEY_C);
    if (cameraModePressed && !gCameraModePressedLastFrame)
    {
        const CameraMode nextMode = cam.GetMode() == CameraMode::Orbit
            ? CameraMode::Free
            : CameraMode::Orbit;
        cam.SetMode(nextMode);
        input.resetMouseState();
        Debugger::cameraModeChanged(nextMode == CameraMode::Free ? "Free" : "Orbit");
    }
    gCameraModePressedLastFrame = cameraModePressed;

    if (!gMouseCaptured)
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

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (!gMouseCaptured)
        return;

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
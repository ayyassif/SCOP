#include "Scop.hpp"

#include <algorithm>
#include <limits>

Camera cam(Vec3(0.0f, 0.0f,  3.0f), Vec3(0.0f, 1.0f,  0.0f));
GLFWwindow *window = initiate();
Input input(window);
namespace
{
    bool gMouseCaptured = true;
    bool gTabPressedLastFrame = false;
    bool gCameraModePressedLastFrame = false;

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
    frameCameraOnVertices(vertices);
    setupVertexBuffers(vertices, indices);
    setupTexture(image);
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
}

void Scop::loop()
{
    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    shader.use();
    shader.setInt("Texture", 0);
    RenderMode renderMode = RenderMode::Color;
    bool mKeyPressedLastFrame = false;
    while(!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, input, cam);

        const bool mKeyPressed = input.getKeyPressed(GLFW_KEY_M);
        if (mKeyPressed && !mKeyPressedLastFrame)
        {
            renderMode = static_cast<RenderMode>((static_cast<int>(renderMode) + 1) % sizeof(RenderMode));
            Debugger::renderModeChanged(renderNames[static_cast<int>(renderMode)]);
        }
        mKeyPressedLastFrame = mKeyPressed;

        // rendering commands
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setInt("renderMode", static_cast<int>(renderMode));

        constexpr float nearPlane = 0.1f;
        constexpr float farPlane = 1000.0f;
        Mat4 projection = Mat4::perspective(
            Math::radians(cam.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
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
    glfwTerminate();
}



 // <-------------------------------------------------------------------------------------->
 // <-----------------------------------helper functions----------------------------------->
 // <-------------------------------------------------------------------------------------->


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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback); 
    glEnable(GL_DEPTH_TEST);
    
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

void processInput(GLFWwindow *window, Input &input, Camera &cam)
{
    const float deltaTime = Math::getDeltaTime();

    if (input.getKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);

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
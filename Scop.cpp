#include "Scop.hpp"

Camera cam(Vec3(0.0f, 0.0f,  3.0f), Vec3(0.0f, 1.0f,  0.0f));
GLFWwindow *window = initiate();
Input input(window);

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

void processInput(GLFWwindow *window, Input &input, Camera &cam)
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
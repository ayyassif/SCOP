#include "Scop.hpp"

Camera cam(Vec3(0.0f, 0.0f,  3.0f), Vec3(0.0f, 1.0f,  0.0f));
GLFWwindow *window = initiate();
Input input(window);



 // <------------------------------------------------------------------------------------->
 // <-----------------------------------class functions----------------------------------->
 // <------------------------------------------------------------------------------------->

Scop::Scop(
    const std::vector<Vertex>& vertices,
    const std::vector<int>& indices,
    const PPM &image
)
{
    indexCount = indices.size();
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
    const std::vector<int>& indices
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
        indices.size() * sizeof(int),
        indices.data(), GL_STATIC_DRAW
    );

    // will change this code later if we are adding colors
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          reinterpret_cast<void*>(offsetof(Vertex, u)));
    glEnableVertexAttribArray(1);
    //  <---->
}

void Scop::loop()
{
    Shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");

    shader.use();
    shader.setInt("Texture", 0);
    while(!glfwWindowShouldClose(window))
    {
        // input
        processInput(window, input, cam);

        // rendering commands
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
       
        Mat4 projection = Mat4::perspective(Math::radians(cam.Zoom), (float)SCR_WIDTH/(float)SCR_HEIGHT, 0.1f, 100.0f);
        
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        Mat4 model = Mat4::model(Vec3( 0.0f,  0.0f,  0.0f), Math::radians(0), Vec3(1.0f, 0.3f, 0.5f));

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
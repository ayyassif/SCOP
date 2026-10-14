#include "Scop.hpp"
#include "parser/OBJParser.hpp"
#include "parser/UVMapper.hpp"
#include "parser/MeshBuilder.hpp"

int main(int ac, char *av[])
{
    try
    {
        Debugger::printWelcome();
        DEBUG::INFO << std::endl;

        if (ac != 3)
            throw std::invalid_argument("Input must be a valid .obj file and a valid .ppm file.");
    
        window = initiate();
        if (window == nullptr)
            throw std::runtime_error("Window initialization failed");
        input = Input(window);

        DEBUG::INFO << "Loading texture: " << av[2] << std::endl;
        PPM image;
        image.load(av[2]);
        DEBUG::SUCCESS << "Texture loaded (" << image.getWidth() << "x"
                       << image.getHeight() << ")" << std::endl;

        DEBUG::INFO << "Loading model: " << av[1] << std::endl;
        OBJParser parser;
        OBJData data = parser.parse(av[1]);
        DEBUG::SUCCESS << "Model loaded (" << data.positions.size() << " vertices, "
                       << data.faces.size() << " faces)" << std::endl;

        UVMapper uvMapper;
        uvMapper.generate(data);
        DEBUG::SUCCESS << "UV charts generated" << std::endl;

        MeshBuilder builder;
        const Mesh mesh = builder.build(data);
        DEBUG::SUCCESS << "Mesh built"
                       << std::endl;

        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        const std::size_t offset = vertices.size();
        vertices.insert(vertices.end(), mesh.vertices.begin(), mesh.vertices.end());

        for (unsigned int index : mesh.indices)
            indices.push_back(static_cast<unsigned int>(index + offset));

        DEBUG::INFO << "Uploading " << vertices.size() << " vertices and "
                << indices.size() << " indices" << std::endl;

        {
            Scop scop(mesh, image);

            Debugger::printControls();
            DEBUG::INFO << std::endl;
            Debugger::printAvailableModes();
            DEBUG::INFO << std::endl;

            scop.loop();
        }

        glfwDestroyWindow(window);
        window = nullptr;
        glfwTerminate();
        DEBUG::SUCCESS << "Application closed normally" << std::endl;
    }
    catch (const std::exception& error)
    {
        DEBUG::ERROR << error.what() << std::endl;
        if (window != nullptr)
        {
            glfwDestroyWindow(window);
            window = nullptr;
            glfwTerminate();
        }
        return 1;
    }
    catch (...)
    {
        DEBUG::ERROR << "Unknown error" << std::endl;
        if (window != nullptr)
        {
            glfwDestroyWindow(window);
            window = nullptr;
            glfwTerminate();
        }
        return 1;
    }

    return 0;
}


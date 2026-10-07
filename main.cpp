#include "Scop.hpp"
#include "parser/OBJParser.hpp"
#include "parser/UVMapper.hpp"
#include "parser/MeshBuilder.hpp"

int main()
{
    PPM image;
    image.load("textures/awesomeface.ppm");

    OBJParser parser;
    OBJData data = parser.parse("resources/teapot2.obj");

    UVMapper uvMapper;
    uvMapper.generate(data);

    MeshBuilder builder;
    const Mesh mesh = builder.build(data);

    std::vector<Vertex> vertices;
    std::vector<int> indices;

    for (const SubMesh& subMesh : mesh.subMeshes)
    {
        const std::size_t offset = vertices.size();
        vertices.insert(vertices.end(), subMesh.vertices.begin(), subMesh.vertices.end());

        for (unsigned int index : subMesh.indices)
            indices.push_back(static_cast<int>(index + offset));
    }

    Scop scop(vertices, indices, image);
    scop.loop();

    return 0;
}


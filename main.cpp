#include "Scop.hpp"
#include "parser/OBJParser.hpp"
#include "parser/UVMapper.hpp"
#include "parser/MeshBuilder.hpp"

int main()
{
    PPM image;
    image.load("textures/spincat.ppm");

    OBJParser parser;
    OBJData data = parser.parse("resources/spincat_fixed.obj");

    UVMapper uvMapper;
    uvMapper.generate(data);

    MeshBuilder builder;
    const Mesh mesh = builder.build(data);

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    for (const SubMesh& subMesh : mesh.subMeshes)
    {
        const std::size_t offset = vertices.size();
        vertices.insert(vertices.end(), subMesh.vertices.begin(), subMesh.vertices.end());

        for (unsigned int index : subMesh.indices)
            indices.push_back(static_cast<unsigned int>(index + offset));
    }

    Scop scop(vertices, indices, image);
    scop.loop();

    return 0;
}


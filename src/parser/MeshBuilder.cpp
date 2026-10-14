#include "MeshBuilder.hpp"

#include <map>
#include <stdexcept>
#include <string>

namespace
{
    struct VertexKey
    {
        int position;
        int texcoord;
        int chartIndex;

        bool operator<(const VertexKey& other) const
        {
            return position < other.position ||
                (position == other.position && (texcoord < other.texcoord ||
                    (texcoord == other.texcoord && chartIndex < other.chartIndex)));
        }
    };

    void validatePosition(const OBJData& data, int index, std::size_t faceIndex,
                          std::size_t cornerIndex)
    {
        if (index < 0 || index >= static_cast<int>(data.positions.size()))
            throw std::runtime_error("MeshBuilder: face " + std::to_string(faceIndex) +
                ", corner " + std::to_string(cornerIndex) +
                " has an invalid position index " + std::to_string(index));
    }

    int resolveTexcoord(const OBJData& data, int index, std::size_t faceIndex,
                        std::size_t cornerIndex)
    {
        if (index < 0)
            return -1;
        if (index >= static_cast<int>(data.texcoords.size()))
            throw std::runtime_error("MeshBuilder: face " + std::to_string(faceIndex) +
                ", corner " + std::to_string(cornerIndex) +
                " has an invalid texcoord index " + std::to_string(index));
        return index;
    }

    unsigned int vertexIndex(Mesh& mesh, std::map<VertexKey, unsigned int>& vertices,
                             const OBJData& data, const OBJIndex& corner,
                             std::size_t faceIndex, std::size_t cornerIndex)
    {
        validatePosition(data, corner.position, faceIndex, cornerIndex);
        const int texcoord = resolveTexcoord(data, corner.texcoord, faceIndex, cornerIndex);
        const int chartIndex = data.faces[faceIndex].chartIndex;
        const VertexKey key{corner.position, texcoord, chartIndex};
        const auto existing = vertices.find(key);
        if (existing != vertices.end())
            return existing->second;

        const Vec3& position = data.positions[corner.position];
        const Vec2 uv = texcoord >= 0 ? data.texcoords[texcoord] : Vec2();
        const unsigned int index = static_cast<unsigned int>(mesh.vertices.size());
        mesh.vertices.push_back(Vertex{position, uv, chartIndex});
        vertices[key] = index;
        return index;
    }
}

Mesh MeshBuilder::build(const OBJData& data)
{
    Mesh mesh;
    std::map<VertexKey, unsigned int> vertexMap;

    for (std::size_t faceIndex = 0; faceIndex < data.faces.size(); ++faceIndex)
    {
        const OBJFace& face = data.faces[faceIndex];
        if (face.vertices.size() < 3)
            throw std::runtime_error("MeshBuilder: face " + std::to_string(faceIndex) +
                " has fewer than three vertices");

        const unsigned int first = vertexIndex(mesh, vertexMap, data, face.vertices[0],
                                               faceIndex, 0);

        for (std::size_t corner = 1; corner + 1 < face.vertices.size(); ++corner)
        {
            const unsigned int second = vertexIndex(mesh, vertexMap, data,
                                                    face.vertices[corner], faceIndex, corner);
            const unsigned int third = vertexIndex(mesh, vertexMap, data,
                                                   face.vertices[corner + 1], faceIndex,
                                                   corner + 1);
            mesh.indices.push_back(first);
            mesh.indices.push_back(second);
            mesh.indices.push_back(third);
        }
    }

    return mesh;
}
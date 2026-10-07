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

        bool operator<(const VertexKey& other) const
        {
            return position < other.position ||
                (position == other.position && texcoord < other.texcoord);
        }
    };

    std::size_t findSubMesh(const Mesh& mesh, const std::string& materialName)
    {
        for (std::size_t i = 0; i < mesh.subMeshes.size(); ++i)
            if (mesh.subMeshes[i].materialName == materialName)
                return i;

        return mesh.subMeshes.size();
    }

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

    unsigned int vertexIndex(SubMesh& subMesh, std::map<VertexKey, unsigned int>& vertices,
                             const OBJData& data, const OBJIndex& corner,
                             std::size_t faceIndex, std::size_t cornerIndex)
    {
        validatePosition(data, corner.position, faceIndex, cornerIndex);
        const int texcoord = resolveTexcoord(data, corner.texcoord, faceIndex, cornerIndex);
        const VertexKey key{corner.position, texcoord};
        const auto existing = vertices.find(key);
        if (existing != vertices.end())
            return existing->second;

        const Vec3& position = data.positions[corner.position];
        const Vec2 uv = texcoord >= 0 ? data.texcoords[texcoord] : Vec2();
        const unsigned int index = static_cast<unsigned int>(subMesh.vertices.size());
        subMesh.vertices.push_back(Vertex{position, uv});
        vertices[key] = index;
        return index;
    }
}

Mesh MeshBuilder::build(const OBJData& data)
{
    Mesh mesh;
    std::vector<std::map<VertexKey, unsigned int>> vertexMaps;

    for (std::size_t faceIndex = 0; faceIndex < data.faces.size(); ++faceIndex)
    {
        const OBJFace& face = data.faces[faceIndex];
        if (face.vertices.size() < 3)
            throw std::runtime_error("MeshBuilder: face " + std::to_string(faceIndex) +
                " has fewer than three vertices");

        std::size_t subMeshIndex = findSubMesh(mesh, face.materialName);
        if (subMeshIndex == mesh.subMeshes.size())
        {
            mesh.subMeshes.push_back(SubMesh());
            mesh.subMeshes.back().materialName = face.materialName;
            vertexMaps.push_back(std::map<VertexKey, unsigned int>());
        }

        SubMesh& subMesh = mesh.subMeshes[subMeshIndex];
        std::map<VertexKey, unsigned int>& vertices = vertexMaps[subMeshIndex];
        const unsigned int first = vertexIndex(subMesh, vertices, data, face.vertices[0],
                                               faceIndex, 0);

        for (std::size_t corner = 1; corner + 1 < face.vertices.size(); ++corner)
        {
            const unsigned int second = vertexIndex(subMesh, vertices, data,
                                                    face.vertices[corner], faceIndex, corner);
            const unsigned int third = vertexIndex(subMesh, vertices, data,
                                                   face.vertices[corner + 1], faceIndex,
                                                   corner + 1);
            subMesh.indices.push_back(first);
            subMesh.indices.push_back(second);
            subMesh.indices.push_back(third);
        }
    }

    return mesh;
}
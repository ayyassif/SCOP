#include "UVMapper.hpp"

#include "../math/Math.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <vector>

namespace
{
	const float	 chartAngle = Math::radians(68.34f);
	const Vec3	 fallback(0.0f, 0.0f, 1.0f);

	struct EdgeKey
	{
		int first;
		int second;

		bool operator<(const EdgeKey& other) const
		{
			return first < other.first || (first == other.first && second < other.second);
		}
	};

	Vec3 faceNormal(const OBJData& data, const OBJFace& face)
	{
		Vec3 normal;
		for (std::size_t i = 0; i < face.vertices.size(); ++i)
		{
			const Vec3& current = data.positions[face.vertices[i].position];
			const Vec3& next = data.positions[face.vertices[(i + 1) % face.vertices.size()].position];
			normal += Vec3((current.y - next.y) * (current.z + next.z),
				(current.z - next.z) * (current.x + next.x),
				(current.x - next.x) * (current.y + next.y));
		}
		if (normal.length() == 0.0f)
			return Vec3(0.0f, 0.0f, 1.0f);
		return normal.normalize();
	}

	bool validTexcoord(const OBJData& data, int index)
	{
		return index >= 0 && index < static_cast<int>(data.texcoords.size());
	}

	struct UVGenerationData
	{
		std::vector<bool> needsUV;
		std::vector<Vec3> faceNormals;
	};

	UVGenerationData collectUVGenerationData(const OBJData& data)
	{
		UVGenerationData result;
		result.needsUV.assign(data.faces.size(), false);
		result.faceNormals.assign(data.faces.size(), Vec3(0.0f, 0.0f, 0.0f));
		for (std::size_t faceIndex = 0; faceIndex < data.faces.size(); ++faceIndex)
		{
			const OBJFace& face = data.faces[faceIndex];
			for (const OBJIndex& corner : face.vertices)
			{
				if (!validTexcoord(data, corner.texcoord))
				{
					result.needsUV[faceIndex] = true;
					break;
				}
			}
			if (result.needsUV[faceIndex])
				result.faceNormals[faceIndex] = faceNormal(data, face);
		}
		return result;
	}

	Charts buildCharts(const OBJData& data,
		const std::vector<bool>& needsUV, const std::vector<Vec3>& faceNormals)
	{
		std::map<EdgeKey, std::vector<int>> edgeMap;
		for (std::size_t faceIndex = 0; faceIndex < data.faces.size(); ++faceIndex)
		{
			if (!needsUV[faceIndex])
				continue;

			const OBJFace& face = data.faces[faceIndex];
			for (std::size_t corner = 0; corner < face.vertices.size(); ++corner)
			{
				const std::size_t next = (corner + 1) % face.vertices.size();
				const int first = face.vertices[corner].position;
				const int second = face.vertices[next].position;
				const EdgeKey key{std::min(first, second), std::max(first, second)};
				edgeMap[key].push_back(static_cast<int>(faceIndex));
			}
		}

		std::vector<std::vector<int>> adjacency(data.faces.size());
		for (const auto& entry : edgeMap)
		{
			const std::vector<int>& faces = entry.second;
			if (faces.size() != 2)
				continue;

			const int firstFace = faces[0];
			const int secondFace = faces[1];
			const float dot = faceNormals[firstFace].dot(faceNormals[secondFace]);
			if (dot >= std::cos(chartAngle))
			{
				adjacency[firstFace].push_back(secondFace);
				adjacency[secondFace].push_back(firstFace);
			}
		}

		std::vector<int> chartId(data.faces.size(), -1);
		Charts charts;
		for (std::size_t faceIndex = 0; faceIndex < data.faces.size(); ++faceIndex)
		{
			if (!needsUV[faceIndex] || chartId[faceIndex] != -1)
				continue;

			std::vector<int> pendingFaces(1, static_cast<int>(faceIndex));
			chartId[faceIndex] = static_cast<int>(charts.size());
			std::vector<int> chart;
			while (!pendingFaces.empty())
			{
				const int current = pendingFaces.back();
				pendingFaces.pop_back();
				chart.push_back(current);
				for (int neighbor : adjacency[current])
				{
					if (!needsUV[neighbor] || chartId[neighbor] != -1)
						continue;
					chartId[neighbor] = chartId[faceIndex];
					pendingFaces.push_back(neighbor);
				}
			}
			charts.push_back(chart);
		}
		return charts;
	}

	void generateChartUVs(OBJData& data, const std::vector<int>& chartFaces,
		const std::vector<Vec3>& faceNormals)
	{
		Vec3 chartNormal;
		for (int faceIndex : chartFaces)
			chartNormal += faceNormals[faceIndex];
		if (chartNormal.length() <= 1e-6f)
			chartNormal = Vec3(0.0f, 0.0f, 1.0f);
		chartNormal = chartNormal.normalize(fallback);

		const Vec3 reference = std::fabs(chartNormal.x) > std::fabs(chartNormal.y)
			? Vec3(0.0f, 1.0f, 0.0f)
			: Vec3(1.0f, 0.0f, 0.0f);
		const Vec3 tangent = (reference - chartNormal * chartNormal.dot(reference)).normalize(fallback);
		const Vec3 bitangent = (chartNormal.cross(tangent)).normalize(fallback);

		std::map<int, Vec2> uvByPosition;
		std::vector<int> uniquePositions;
		for (int faceIndex : chartFaces)
		{
			for (const OBJIndex& corner : data.faces[faceIndex].vertices)
			{
				if (uvByPosition.find(corner.position) != uvByPosition.end())
					continue;
				uniquePositions.push_back(corner.position);
				uvByPosition[corner.position] = Vec2();
			}
		}

		Vec3 centroid;
		for (int positionIndex : uniquePositions)
			centroid += data.positions[positionIndex];
		centroid = centroid * (1.0f / std::max(1.0f, static_cast<float>(uniquePositions.size())));

		float minU = std::numeric_limits<float>::max();
		float maxU = -std::numeric_limits<float>::max();
		float minV = std::numeric_limits<float>::max();
		float maxV = -std::numeric_limits<float>::max();
		for (int positionIndex : uniquePositions)
		{
			const Vec3 delta = data.positions[positionIndex] - centroid;
			const Vec2 projected(delta.dot(tangent), delta.dot(bitangent));
			minU = std::min(minU, projected.x);
			maxU = std::max(maxU, projected.x);
			minV = std::min(minV, projected.y);
			maxV = std::max(maxV, projected.y);
			uvByPosition[positionIndex] = projected;
		}

		const float rangeU = std::max(maxU - minU, 1.0e-4f);
		const float rangeV = std::max(maxV - minV, 1.0e-4f);
		for (int positionIndex : uniquePositions)
		{
			const Vec2 projected = uvByPosition[positionIndex];
			const Vec2 normalized((projected.x - minU) / rangeU, (projected.y - minV) / rangeV);
			uvByPosition[positionIndex] = Vec2(1.0f - normalized.y, normalized.x);
		}
		// <<<<------------------->>>
		std::map<int, int> texcoordIndexByPosition;
		for (int faceIndex : chartFaces)
		{
			OBJFace& face = data.faces[faceIndex];
			for (OBJIndex& corner : face.vertices)
			{
				if (validTexcoord(data, corner.texcoord))
					continue;
				if (texcoordIndexByPosition.find(corner.position) == texcoordIndexByPosition.end())
				{
					texcoordIndexByPosition[corner.position] = static_cast<int>(data.texcoords.size());
					data.texcoords.push_back(uvByPosition[corner.position]);
				}
				corner.texcoord = texcoordIndexByPosition[corner.position];
			}
		}
	}
}

void UVMapper::generate(OBJData& data)
{
	const UVGenerationData generationData = collectUVGenerationData(data);

	bool anyMissing = false;
	for (bool flag : generationData.needsUV)
		anyMissing = anyMissing || flag;
	if (!anyMissing)
		return;

	const Charts charts = buildCharts(data, generationData.needsUV, generationData.faceNormals);

	for (std::size_t chartIndex = 0; chartIndex < charts.size(); ++chartIndex)
	{
		const std::vector<int>& chartFaces = charts[chartIndex];
		for (int faceIndex : chartFaces)
			data.faces[faceIndex].chartIndex = static_cast<int>(chartIndex);
		generateChartUVs(data, chartFaces, generationData.faceNormals);
	}
}

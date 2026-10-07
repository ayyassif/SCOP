#include "UVMapper.hpp"

#include "../math/Math.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>
#include <map>
#include <utility>
#include <vector>

namespace
{
	const float chartAngle = Math::radians(55.0f);
	const float atlasMargin = 0.04f;
	const double lscmEpsilon = 1e-12;
	const double solveEpsilon = 1e-14;

	struct EdgeKey
	{
		int first;
		int second;

		bool operator<(const EdgeKey& other) const
		{
			return first < other.first || (first == other.first && second < other.second);
		}
	};

	struct EdgeUse
	{
		int face;
		int firstCorner;
		int secondCorner;
	};

	struct Chart
	{
		std::vector<int> faces;
		Vec3 normal;
	};

	struct ChartVertex
	{
		int position = -1;
		Vec3 positionValue;
		bool hasFixedUV = false;
		Vec2 fixedUV;
		Vec2 anchorUV;
		int unknownIndex = -1;
		int texcoordIndex = -1;
	};

	struct TriangulatedTriangle
	{
		std::array<int, 3> positions;
	};

	struct SparseRow
	{
		std::vector<std::pair<int, double>> coefficients;
		double rhs = 0.0;
	};

	struct ChartSolution
	{
		std::vector<ChartVertex> vertices;
		std::map<int, int> vertexByPosition;
		std::vector<Vec2> solvedUVs;
		bool hasExistingUVs = false;
		bool packIntoAtlas = false;
	};

	double squaredDistance(const Vec3& first, const Vec3& second)
	{
		const double dx = static_cast<double>(first.x - second.x);
		const double dy = static_cast<double>(first.y - second.y);
		const double dz = static_cast<double>(first.z - second.z);
		return dx * dx + dy * dy + dz * dz;
	}

	double cross2(const Vec2& first, const Vec2& second)
	{
		return static_cast<double>(first.x) * static_cast<double>(second.y) -
			static_cast<double>(first.y) * static_cast<double>(second.x);
	}

	Vec3 safeNormalize(const Vec3& value, const Vec3& fallback = Vec3(0.0f, 0.0f, 1.0f))
	{
		const float length = value.length();
		if (length <= 0.0f)
			return fallback;
		return value / length;
	}

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

	Vec3 chartNormal(const std::vector<int>& faces, const std::vector<Vec3>& normals)
	{
		Vec3 result;
		for (int face : faces)
			result += normals[face];
		if (result.length() == 0.0f)
			return Vec3(0.0f, 0.0f, 1.0f);
		return result.normalize();
	}

	bool validTexcoord(const OBJData& data, int index)
	{
		return index >= 0 && index < static_cast<int>(data.texcoords.size());
	}

	bool faceNeedsUV(const OBJData& data, const OBJFace& face)
	{
		for (const OBJIndex& corner : face.vertices)
			if (!validTexcoord(data, corner.texcoord))
				return true;
		return false;
	}

	bool edgeCanBeSmooth(const OBJData& data, const EdgeUse& first, const EdgeUse& second)
	{
		const OBJFace& firstFace = data.faces[first.face];
		const OBJFace& secondFace = data.faces[second.face];
		return !validTexcoord(data, firstFace.vertices[first.firstCorner].texcoord) &&
			!validTexcoord(data, firstFace.vertices[first.secondCorner].texcoord) &&
			!validTexcoord(data, secondFace.vertices[second.firstCorner].texcoord) &&
			!validTexcoord(data, secondFace.vertices[second.secondCorner].texcoord);
	}

	std::vector<Vec2> projectFaceTo2D(const OBJData& data, const OBJFace& face)
	{
		const Vec3 normal = faceNormal(data, face);
		const Vec3 absolute(std::fabs(normal.x), std::fabs(normal.y), std::fabs(normal.z));
		std::vector<Vec2> projected;
		projected.reserve(face.vertices.size());
		for (const OBJIndex& corner : face.vertices)
		{
			const Vec3& position = data.positions[corner.position];
			if (absolute.x >= absolute.y && absolute.x >= absolute.z)
				projected.emplace_back(position.y, position.z);
			else if (absolute.y >= absolute.z)
				projected.emplace_back(position.x, position.z);
			else
				projected.emplace_back(position.x, position.y);
		}
		return projected;
	}

	double polygonSignedArea(const std::vector<Vec2>& polygon)
	{
		double area = 0.0;
		for (std::size_t i = 0; i < polygon.size(); ++i)
		{
			const Vec2& current = polygon[i];
			const Vec2& next = polygon[(i + 1) % polygon.size()];
			area += cross2(current, next);
		}
		return area * 0.5;
	}

	bool pointInTriangle(const Vec2& point, const Vec2& a, const Vec2& b, const Vec2& c, bool counterClockwise)
	{
		const double ab = cross2(b - a, point - a);
		const double bc = cross2(c - b, point - b);
		const double ca = cross2(a - c, point - c);
		const double tolerance = 1e-10;
		if (counterClockwise)
			return ab >= -tolerance && bc >= -tolerance && ca >= -tolerance;
		return ab <= tolerance && bc <= tolerance && ca <= tolerance;
	}

	std::vector<TriangulatedTriangle> triangulateFace(const OBJData& data, const OBJFace& face)
	{
		std::vector<TriangulatedTriangle> triangles;
		if (face.vertices.size() < 3)
			return triangles;
		if (face.vertices.size() == 3)
		{
			triangles.push_back({{face.vertices[0].position, face.vertices[1].position, face.vertices[2].position}});
			return triangles;
		}

		const std::vector<Vec2> polygon = projectFaceTo2D(data, face);
		if (polygon.size() != face.vertices.size())
			return triangles;

		const double area = polygonSignedArea(polygon);
		if (std::fabs(area) <= lscmEpsilon)
			return triangles;

		std::vector<int> remaining(face.vertices.size());
		for (std::size_t i = 0; i < remaining.size(); ++i)
			remaining[i] = static_cast<int>(i);

		const bool counterClockwise = area > 0.0;
		int guard = 0;
		while (remaining.size() > 3 && guard < 1024)
		{
			bool clipped = false;
			for (std::size_t i = 0; i < remaining.size(); ++i)
			{
				const int previous = remaining[(i + remaining.size() - 1) % remaining.size()];
				const int current = remaining[i];
				const int next = remaining[(i + 1) % remaining.size()];
				const Vec2& a = polygon[previous];
				const Vec2& b = polygon[current];
				const Vec2& c = polygon[next];
				const double turn = cross2(b - a, c - b);
				if (counterClockwise ? turn <= lscmEpsilon : turn >= -lscmEpsilon)
					continue;

				bool containsOtherPoint = false;
				for (int vertex : remaining)
				{
					if (vertex == previous || vertex == current || vertex == next)
						continue;
					if (pointInTriangle(polygon[vertex], a, b, c, counterClockwise))
					{
						containsOtherPoint = true;
						break;
					}
				}
				if (containsOtherPoint)
					continue;

				triangles.push_back({{face.vertices[previous].position,
					face.vertices[current].position,
					face.vertices[next].position}});
				remaining.erase(remaining.begin() + static_cast<std::ptrdiff_t>(i));
				clipped = true;
				break;
			}
			if (!clipped)
				break;
			++guard;
		}

		if (remaining.size() == 3)
			triangles.push_back({{face.vertices[remaining[0]].position,
				face.vertices[remaining[1]].position,
				face.vertices[remaining[2]].position}});

		if (triangles.empty())
		{
			for (std::size_t i = 1; i + 1 < face.vertices.size(); ++i)
				triangles.push_back({{face.vertices[0].position,
					face.vertices[i].position,
					face.vertices[i + 1].position}});
		}

		return triangles;
	}

	std::vector<ChartVertex> buildChartVertices(const OBJData& data, const Chart& chart, std::map<int, int>& vertexByPosition, bool& hasExistingUVs)
	{
		std::vector<ChartVertex> vertices;
		hasExistingUVs = false;
		for (int faceIndex : chart.faces)
		{
			const OBJFace& face = data.faces[faceIndex];
			for (const OBJIndex& corner : face.vertices)
			{
				auto found = vertexByPosition.find(corner.position);
				if (found == vertexByPosition.end())
				{
					ChartVertex vertex;
					vertex.position = corner.position;
					vertex.positionValue = data.positions[corner.position];
					const int index = static_cast<int>(vertices.size());
					vertexByPosition[corner.position] = index;
					vertices.push_back(vertex);
					found = vertexByPosition.find(corner.position);
				}
				ChartVertex& vertex = vertices[found->second];
				if (validTexcoord(data, corner.texcoord))
				{
					if (!vertex.hasFixedUV)
					{
						vertex.hasFixedUV = true;
						vertex.fixedUV = data.texcoords[corner.texcoord];
						vertex.texcoordIndex = corner.texcoord;
						vertex.anchorUV = vertex.fixedUV;
					}
					hasExistingUVs = true;
				}
			}
		}
		return vertices;
	}

	std::pair<int, int> chooseAnchors(const std::vector<ChartVertex>& vertices)
	{
		std::vector<int> fixed;
		for (std::size_t i = 0; i < vertices.size(); ++i)
			if (vertices[i].hasFixedUV)
				fixed.push_back(static_cast<int>(i));

		auto farthestPair = [&](const std::vector<int>& candidates) -> std::pair<int, int>
		{
			if (candidates.size() < 2)
				return std::make_pair(-1, -1);
			double bestDistance = -1.0;
			std::pair<int, int> best(-1, -1);
			for (std::size_t i = 0; i < candidates.size(); ++i)
				for (std::size_t j = i + 1; j < candidates.size(); ++j)
				{
					const double distance = squaredDistance(vertices[candidates[i]].positionValue, vertices[candidates[j]].positionValue);
					if (distance > bestDistance)
					{
						bestDistance = distance;
						best = std::make_pair(candidates[i], candidates[j]);
					}
				}
			return best;
		};

		if (fixed.size() >= 2)
			return farthestPair(fixed);

		if (fixed.size() == 1)
		{
			int bestOther = -1;
			double bestDistance = -1.0;
			for (std::size_t i = 0; i < vertices.size(); ++i)
			{
				if (static_cast<int>(i) == fixed.front())
					continue;
				const double distance = squaredDistance(vertices[fixed.front()].positionValue, vertices[i].positionValue);
				if (distance > bestDistance)
				{
					bestDistance = distance;
					bestOther = static_cast<int>(i);
				}
			}
			return std::make_pair(fixed.front(), bestOther);
		}

		std::vector<int> all(vertices.size());
		for (std::size_t i = 0; i < vertices.size(); ++i)
			all[i] = static_cast<int>(i);
		return farthestPair(all);
	}

	Vec2 fallbackProjectVertex(const Vec3& position, const Vec3& origin, const Vec3& tangent, const Vec3& bitangent)
	{
		return Vec2((position - origin).dot(tangent), (position - origin).dot(bitangent));
	}

	bool solveDenseSystem(std::vector<double> matrix, std::vector<double> rhs, std::vector<double>& solution)
	{
		const int n = static_cast<int>(rhs.size());
		for (int pivotIndex = 0; pivotIndex < n; ++pivotIndex)
		{
			int pivotRow = pivotIndex;
			double pivotValue = std::fabs(matrix[static_cast<std::size_t>(pivotIndex) * n + pivotIndex]);
			for (int row = pivotIndex + 1; row < n; ++row)
			{
				const double value = std::fabs(matrix[static_cast<std::size_t>(row) * n + pivotIndex]);
				if (value > pivotValue)
				{
					pivotValue = value;
					pivotRow = row;
				}
			}
			if (pivotValue < solveEpsilon)
				return false;

			if (pivotRow != pivotIndex)
			{
				for (int column = pivotIndex; column < n; ++column)
					std::swap(matrix[static_cast<std::size_t>(pivotIndex) * n + column], matrix[static_cast<std::size_t>(pivotRow) * n + column]);
				std::swap(rhs[pivotIndex], rhs[pivotRow]);
			}

			const double pivot = matrix[static_cast<std::size_t>(pivotIndex) * n + pivotIndex];
			for (int row = pivotIndex + 1; row < n; ++row)
			{
				const double factor = matrix[static_cast<std::size_t>(row) * n + pivotIndex] / pivot;
				if (std::fabs(factor) < solveEpsilon)
					continue;
				matrix[static_cast<std::size_t>(row) * n + pivotIndex] = 0.0;
				for (int column = pivotIndex + 1; column < n; ++column)
					matrix[static_cast<std::size_t>(row) * n + column] -= factor * matrix[static_cast<std::size_t>(pivotIndex) * n + column];
				rhs[row] -= factor * rhs[pivotIndex];
			}
		}

		solution.assign(n, 0.0);
		for (int row = n - 1; row >= 0; --row)
		{
			double sum = rhs[row];
			for (int column = row + 1; column < n; ++column)
				sum -= matrix[static_cast<std::size_t>(row) * n + column] * solution[column];
			const double diagonal = matrix[static_cast<std::size_t>(row) * n + row];
			if (std::fabs(diagonal) < solveEpsilon)
				return false;
			solution[row] = sum / diagonal;
		}
		return true;
	}

	bool solveLSCM(const OBJData& data, const Chart& chart, ChartSolution& solution)
	{
		solution.vertexByPosition.clear();
		solution.vertices = buildChartVertices(data, chart, solution.vertexByPosition, solution.hasExistingUVs);
		solution.packIntoAtlas = !solution.hasExistingUVs;
		solution.solvedUVs.assign(solution.vertices.size(), Vec2());

		if (solution.vertices.empty())
			return true;

		std::vector<TriangulatedTriangle> triangles;
		triangles.reserve(chart.faces.size() * 2);
		for (int faceIndex : chart.faces)
		{
			const std::vector<TriangulatedTriangle> faceTriangles = triangulateFace(data, data.faces[faceIndex]);
			triangles.insert(triangles.end(), faceTriangles.begin(), faceTriangles.end());
		}

		const std::pair<int, int> anchors = chooseAnchors(solution.vertices);
		if (anchors.first < 0 || anchors.second < 0)
			return false;

		// Two anchors remove the similarity ambiguity of the LSCM system. The first anchor defines the
		// origin of the local UV frame and the second anchor provides a stable baseline direction and
		// length so the least-squares solve has a concrete gauge even before atlas packing.
		if (!solution.vertices[anchors.first].hasFixedUV)
		{
			solution.vertices[anchors.first].hasFixedUV = true;
			solution.vertices[anchors.first].anchorUV = Vec2(0.0f, 0.0f);
		}
		else
			solution.vertices[anchors.first].anchorUV = solution.vertices[anchors.first].fixedUV;

		if (!solution.vertices[anchors.second].hasFixedUV)
		{
			const double baseline = std::sqrt(std::max(squaredDistance(solution.vertices[anchors.first].positionValue, solution.vertices[anchors.second].positionValue), 1e-12));
			solution.vertices[anchors.second].hasFixedUV = true;
			solution.vertices[anchors.second].anchorUV = solution.vertices[anchors.first].anchorUV + Vec2(static_cast<float>(baseline), 0.0f);
		}
		else
			solution.vertices[anchors.second].anchorUV = solution.vertices[anchors.second].fixedUV;

		int unknownCount = 0;
		for (ChartVertex& vertex : solution.vertices)
			if (!vertex.hasFixedUV)
				vertex.unknownIndex = unknownCount++;

		if (unknownCount == 0)
		{
			for (std::size_t i = 0; i < solution.vertices.size(); ++i)
				solution.solvedUVs[i] = solution.vertices[i].anchorUV;
			return true;
		}

		std::vector<SparseRow> rows;
		rows.reserve(triangles.size() * 2 + 4);

		auto addCoefficient = [](SparseRow& row, int variableIndex, double value)
		{
			if (std::fabs(value) < lscmEpsilon)
				return;
			for (auto& coefficient : row.coefficients)
				if (coefficient.first == variableIndex)
				{
					coefficient.second += value;
					return;
				}
			row.coefficients.push_back(std::make_pair(variableIndex, value));
		};

		for (const TriangulatedTriangle& triangle : triangles)
		{
			const int localIndices[3] = {
				solution.vertexByPosition[triangle.positions[0]],
				solution.vertexByPosition[triangle.positions[1]],
				solution.vertexByPosition[triangle.positions[2]]
			};
			const Vec3& p0 = solution.vertices[localIndices[0]].positionValue;
			const Vec3& p1 = solution.vertices[localIndices[1]].positionValue;
			const Vec3& p2 = solution.vertices[localIndices[2]].positionValue;

			const Vec3 edge01 = p1 - p0;
			const Vec3 edge02 = p2 - p0;
			const double length01 = edge01.length();
			if (length01 <= lscmEpsilon)
				continue;

			const Vec3 triangleNormal = edge01.cross(edge02);
			const double normalLength = triangleNormal.length();
			if (normalLength <= lscmEpsilon)
				continue;

			const Vec3 tangent = safeNormalize(edge01);
			const Vec3 bitangent = safeNormalize(triangleNormal.cross(tangent));
			const Vec2 local0(0.0f, 0.0f);
			const Vec2 local1(static_cast<float>(length01), 0.0f);
			const Vec2 local2(static_cast<float>(edge02.dot(tangent)), static_cast<float>(edge02.dot(bitangent)));
			const double twiceArea = cross2(local1 - local0, local2 - local0);
			if (std::fabs(twiceArea) <= lscmEpsilon)
				continue;

			const double areaWeight = std::sqrt(std::fabs(twiceArea) * 0.5);
			std::array<std::pair<double, double>, 3> gradients;
			const std::array<Vec2, 3> local = {local0, local1, local2};
			for (int i = 0; i < 3; ++i)
			{
				const int j = (i + 1) % 3;
				const int k = (i + 2) % 3;
				const double dLambdaDx = (static_cast<double>(local[j].y) - static_cast<double>(local[k].y)) / twiceArea;
				const double dLambdaDy = (static_cast<double>(local[k].x) - static_cast<double>(local[j].x)) / twiceArea;
				gradients[i] = std::make_pair(dLambdaDx, dLambdaDy);
			}

			// Each triangle contributes the discrete Cauchy-Riemann equations. Solving the least-squares
			// system minimizes the conformal residual, which is why LSCM preserves local angles much better
			// than a direct tangent-plane projection.
			for (int equation = 0; equation < 2; ++equation)
			{
				SparseRow row;
				for (int localVertex = 0; localVertex < 3; ++localVertex)
				{
					const ChartVertex& chartVertex = solution.vertices[localIndices[localVertex]];
					const double dLambdaDx = gradients[localVertex].first * areaWeight;
					const double dLambdaDy = gradients[localVertex].second * areaWeight;
					const double coefficientU = equation == 0 ? dLambdaDx : dLambdaDy;
					const double coefficientV = equation == 0 ? -dLambdaDy : dLambdaDx;
					if (chartVertex.hasFixedUV)
					{
						row.rhs -= coefficientU * static_cast<double>(chartVertex.anchorUV.x) +
							coefficientV * static_cast<double>(chartVertex.anchorUV.y);
					}
					else
					{
						const int base = chartVertex.unknownIndex * 2;
						addCoefficient(row, base, coefficientU);
						addCoefficient(row, base + 1, coefficientV);
					}
				}
				if (!row.coefficients.empty())
					rows.push_back(row);
			}
		}

		if (rows.empty())
			return false;

		const int variableCount = unknownCount * 2;
		std::vector<double> normal(static_cast<std::size_t>(variableCount) * variableCount, 0.0);
		std::vector<double> rhs(variableCount, 0.0);
		for (const SparseRow& row : rows)
		{
			for (const auto& lhs : row.coefficients)
			{
				rhs[lhs.first] += lhs.second * row.rhs;
				for (const auto& rhsTerm : row.coefficients)
					normal[static_cast<std::size_t>(lhs.first) * variableCount + rhsTerm.first] += lhs.second * rhsTerm.second;
			}
		}
		for (int diagonal = 0; diagonal < variableCount; ++diagonal)
			normal[static_cast<std::size_t>(diagonal) * variableCount + diagonal] += 1e-12;

		std::vector<double> solutionVector;
		if (!solveDenseSystem(normal, rhs, solutionVector))
			return false;

		for (std::size_t i = 0; i < solution.vertices.size(); ++i)
		{
			const ChartVertex& vertex = solution.vertices[i];
			if (vertex.hasFixedUV)
				solution.solvedUVs[i] = vertex.anchorUV;
			else
			{
				const int base = vertex.unknownIndex * 2;
				solution.solvedUVs[i] = Vec2(static_cast<float>(solutionVector[base]), static_cast<float>(solutionVector[base + 1]));
			}
		}

		return true;
	}

	bool fallbackParameterizeChart(const OBJData& data, const Chart& chart, ChartSolution& solution)
	{
		solution.vertexByPosition.clear();
		solution.vertices = buildChartVertices(data, chart, solution.vertexByPosition, solution.hasExistingUVs);
		solution.packIntoAtlas = !solution.hasExistingUVs;
		solution.solvedUVs.assign(solution.vertices.size(), Vec2());
		if (solution.vertices.empty())
			return true;

		const Vec3 normal = chart.normal.length() == 0.0f ? Vec3(0.0f, 0.0f, 1.0f) : chart.normal;
		const Vec3 reference = std::fabs(normal.x) < 0.8f ? Vec3(1.0f, 0.0f, 0.0f) : Vec3(0.0f, 1.0f, 0.0f);
		const Vec3 tangent = safeNormalize(reference.cross(normal));
		const Vec3 bitangent = safeNormalize(normal.cross(tangent));

		Vec3 origin = solution.vertices.front().positionValue;
		for (const ChartVertex& vertex : solution.vertices)
			if (vertex.hasFixedUV)
			{
				origin = vertex.positionValue;
				break;
			}

		std::vector<Vec2> projected(solution.vertices.size());
		double minU = 0.0;
		double maxU = 0.0;
		double minV = 0.0;
		double maxV = 0.0;
		bool first = true;
		for (std::size_t i = 0; i < solution.vertices.size(); ++i)
		{
			projected[i] = fallbackProjectVertex(solution.vertices[i].positionValue, origin, tangent, bitangent);
			if (solution.vertices[i].hasFixedUV)
				projected[i] = solution.vertices[i].anchorUV;
			if (first)
			{
				minU = maxU = projected[i].x;
				minV = maxV = projected[i].y;
				first = false;
			}
			else
			{
				minU = std::min(minU, static_cast<double>(projected[i].x));
				maxU = std::max(maxU, static_cast<double>(projected[i].x));
				minV = std::min(minV, static_cast<double>(projected[i].y));
				maxV = std::max(maxV, static_cast<double>(projected[i].y));
			}
		}

		const double width = std::max(maxU - minU, 1e-8);
		const double height = std::max(maxV - minV, 1e-8);
		const double scale = 1.0 / std::max(width, height);
		for (std::size_t i = 0; i < solution.vertices.size(); ++i)
		{
			if (solution.vertices[i].hasFixedUV)
				solution.solvedUVs[i] = solution.vertices[i].anchorUV;
			else
				solution.solvedUVs[i] = Vec2(static_cast<float>((projected[i].x - minU) * scale), static_cast<float>((projected[i].y - minV) * scale));
		}

		return true;
	}

	void appendChartUVs(OBJData& data, const Chart& chart, ChartSolution& solution)
	{
		for (std::size_t i = 0; i < solution.vertices.size(); ++i)
		{
			ChartVertex& vertex = solution.vertices[i];
			if (vertex.hasFixedUV)
				continue;
			vertex.texcoordIndex = static_cast<int>(data.texcoords.size());
			data.texcoords.push_back(solution.solvedUVs[i]);
		}

		for (int faceIndex : chart.faces)
		{
			OBJFace& face = data.faces[faceIndex];
			for (OBJIndex& corner : face.vertices)
			{
				if (validTexcoord(data, corner.texcoord))
					continue;
				const int localIndex = solution.vertexByPosition[corner.position];
				corner.texcoord = solution.vertices[localIndex].texcoordIndex;
			}
		}
	}

	void normalizeAndPackChart(OBJData& data, const Chart& chart, ChartSolution& solution, int packableIndex, int packableChartCount)
	{
		if (solution.vertices.empty())
			return;

		if (!solution.packIntoAtlas)
		{
			appendChartUVs(data, chart, solution);
			return;
		}

		double minU = 0.0;
		double maxU = 0.0;
		double minV = 0.0;
		double maxV = 0.0;
		bool first = true;
		for (const Vec2& uv : solution.solvedUVs)
		{
			if (first)
			{
				minU = maxU = uv.x;
				minV = maxV = uv.y;
				first = false;
			}
			else
			{
				minU = std::min(minU, static_cast<double>(uv.x));
				maxU = std::max(maxU, static_cast<double>(uv.x));
				minV = std::min(minV, static_cast<double>(uv.y));
				maxV = std::max(maxV, static_cast<double>(uv.y));
			}
		}

		const int columns = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(std::max(packableChartCount, 1)))));
		const int rows = (packableChartCount + columns - 1) / columns;
		const int column = packableIndex % columns;
		const int row = packableIndex / columns;
		const float cellWidth = 1.0f / static_cast<float>(columns);
		const float cellHeight = 1.0f / static_cast<float>(rows);
		const double width = std::max(maxU - minU, 1e-8);
		const double height = std::max(maxV - minV, 1e-8);

		// LSCM gives a natural local parameterization. Packing applies only a uniform scale and a
		// translation so the chart keeps its aspect ratio while being fit into its atlas cell.
		const double normalizationScale = 1.0 / std::max(width, height);
		const double normalizedWidth = width * normalizationScale;
		const double normalizedHeight = height * normalizationScale;
		const double packScale = std::min((cellWidth - atlasMargin) / normalizedWidth,
			(cellHeight - atlasMargin) / normalizedHeight);
		const double finalScale = normalizationScale * packScale;
		const float scaledWidth = static_cast<float>(width * finalScale);
		const float scaledHeight = static_cast<float>(height * finalScale);
		const float offsetU = column * cellWidth + (cellWidth - scaledWidth) * 0.5f;
		const float offsetV = row * cellHeight + (cellHeight - scaledHeight) * 0.5f;

		for (std::size_t i = 0; i < solution.vertices.size(); ++i)
		{
			const Vec2 raw = solution.solvedUVs[i];
			solution.solvedUVs[i] = Vec2(
				offsetU + static_cast<float>((static_cast<double>(raw.x) - minU) * finalScale),
				offsetV + static_cast<float>((static_cast<double>(raw.y) - minV) * finalScale));
		}

		appendChartUVs(data, chart, solution);
	}
}

void UVMapper::generate(OBJData& data)
{
	std::vector<int> needsUV(data.faces.size(), 0);
	std::vector<Vec3> faceNormals(data.faces.size(), Vec3(0.0f, 0.0f, 0.0f));
	for (std::size_t faceIndex = 0; faceIndex < data.faces.size(); ++faceIndex)
	{
		const OBJFace& face = data.faces[faceIndex];
		for (const OBJIndex& corner : face.vertices)
		{
			if (!validTexcoord(data, corner.texcoord))
			{
				needsUV[faceIndex] = 1;
				break;
			}
		}
		faceNormals[faceIndex] = faceNormal(data, face);
	}

	bool anyMissing = false;
	for (int flag : needsUV)
		anyMissing = anyMissing || (flag != 0);
	if (!anyMissing)
		return;

	std::map<EdgeKey, std::vector<int>> edgeMap;
	for (std::size_t faceIndex = 0; faceIndex < data.faces.size(); ++faceIndex)
	{
		if (needsUV[faceIndex] == 0)
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
		const float threshold = std::cos(chartAngle);
		if (dot >= threshold)
		{
			adjacency[firstFace].push_back(secondFace);
			adjacency[secondFace].push_back(firstFace);
		}
	}

	std::vector<int> chartId(data.faces.size(), -1);
	std::vector<std::vector<int>> charts;
	for (std::size_t faceIndex = 0; faceIndex < data.faces.size(); ++faceIndex)
	{
		if (needsUV[faceIndex] == 0 || chartId[faceIndex] != -1)
			continue;

		std::vector<int> queue;
		queue.push_back(static_cast<int>(faceIndex));
		chartId[faceIndex] = static_cast<int>(charts.size());
		std::vector<int> chart;
		while (!queue.empty())
		{
			const int current = queue.back();
			queue.pop_back();
			chart.push_back(current);
			for (int neighbor : adjacency[current])
			{
				if (needsUV[neighbor] == 0 || chartId[neighbor] != -1)
					continue;
				chartId[neighbor] = chartId[faceIndex];
				queue.push_back(neighbor);
			}
		}
		charts.push_back(chart);
	}

	for (const std::vector<int>& chartFaces : charts)
	{
		if (chartFaces.empty())
			continue;

		Vec3 chartNormal(0.0f, 0.0f, 0.0f);
		for (int faceIndex : chartFaces)
			chartNormal += faceNormals[faceIndex];
		if (chartNormal.length() <= 1e-6f)
			chartNormal = Vec3(0.0f, 0.0f, 1.0f);
		chartNormal = safeNormalize(chartNormal);

		Vec3 reference = std::fabs(chartNormal.x) > std::fabs(chartNormal.y)
			? Vec3(0.0f, 1.0f, 0.0f)
			: Vec3(1.0f, 0.0f, 0.0f);
		Vec3 tangent = safeNormalize(reference - chartNormal * chartNormal.dot(reference));
		Vec3 bitangent = safeNormalize(chartNormal.cross(tangent));

		std::map<int, Vec2> uvByPosition;
		std::vector<int> uniquePositions;
		for (int faceIndex : chartFaces)
		{
			const OBJFace& face = data.faces[faceIndex];
			for (const OBJIndex& corner : face.vertices)
			{
				if (uvByPosition.find(corner.position) != uvByPosition.end())
					continue;
				uniquePositions.push_back(corner.position);
				uvByPosition[corner.position] = Vec2();
			}
		}

		Vec3 centroid(0.0f, 0.0f, 0.0f);
		for (int positionIndex : uniquePositions)
			centroid += data.positions[positionIndex];
		const float invCount = 1.0f / std::max(1.0f, static_cast<float>(uniquePositions.size()));
		centroid = centroid * invCount;

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
			Vec2 projected = uvByPosition[positionIndex];
			Vec2 normalized(
				(projected.x - minU) / rangeU,
				(projected.y - minV) / rangeV);
			// Rotate the texture 90 degrees clockwise in UV space.
			uvByPosition[positionIndex] = Vec2(1.0f - normalized.y, normalized.x);
		}

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

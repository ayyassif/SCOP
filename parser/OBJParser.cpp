#include "OBJParser.hpp"

#include <cstddef>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
	std::string trim(const std::string& value)
	{
		const std::size_t first = value.find_first_not_of(" \t\r\n");
		if (first == std::string::npos)
			return "";

		const std::size_t last = value.find_last_not_of(" \t\r\n");
		return value.substr(first, last - first + 1);
	}

	[[noreturn]] void throwLineError(const std::string& filename,
									 std::size_t lineNumber,
									 const std::string& message)
	{
		throw std::runtime_error(filename + ":" + std::to_string(lineNumber) + ": " + message);
	}

	float parseFloat(const std::string& text,
					 const std::string& filename,
					 std::size_t lineNumber,
					 const std::string& fieldName)
	{
		std::size_t consumed = 0;
		try
		{
			const float value = std::stof(text, &consumed);
			if (consumed != text.size())
				throw std::invalid_argument("trailing characters");
			return value;
		}
		catch (const std::exception&)
		{
			throwLineError(filename, lineNumber, "invalid " + fieldName + " '" + text + "'");
		}
	}

	int parseInteger(const std::string& text,
					 const std::string& filename,
					 std::size_t lineNumber,
					 const std::string& fieldName)
	{
		std::size_t consumed = 0;
		try
		{
			const long long value = std::stoll(text, &consumed);
			if (consumed != text.size() || value < std::numeric_limits<int>::min() ||
				value > std::numeric_limits<int>::max())
				throw std::invalid_argument("invalid integer");
			return static_cast<int>(value);
		}
		catch (const std::exception&)
		{
			throwLineError(filename, lineNumber, "invalid " + fieldName + " '" + text + "'");
		}
	}

	int resolveIndex(int objIndex,
					 std::size_t itemCount,
					 const std::string& filename,
					 std::size_t lineNumber,
					 const std::string& fieldName)
	{
		if (objIndex == 0)
			throwLineError(filename, lineNumber, fieldName + " index cannot be zero");

		const long long resolved = objIndex > 0
			? static_cast<long long>(objIndex) - 1
			: static_cast<long long>(itemCount) + objIndex;

		if (resolved < 0 || resolved >= static_cast<long long>(itemCount))
			throwLineError(filename, lineNumber, fieldName + " index is out of range");

		return static_cast<int>(resolved);
	}

	int parseOptionalIndex(const std::string& text,
						   std::size_t itemCount,
						   const std::string& filename,
						   std::size_t lineNumber,
						   const std::string& fieldName)
	{
		if (text.empty())
			return -1;

		return resolveIndex(parseInteger(text, filename, lineNumber, fieldName),
							itemCount, filename, lineNumber, fieldName);
	}

	OBJIndex parseFaceVertex(const std::string& token,
							 const OBJData& data,
							 const std::string& filename,
							 std::size_t lineNumber)
	{
		const std::size_t firstSlash = token.find('/');
		const std::size_t secondSlash = firstSlash == std::string::npos
			? std::string::npos
			: token.find('/', firstSlash + 1);

		if (secondSlash != std::string::npos && token.find('/', secondSlash + 1) != std::string::npos)
			throwLineError(filename, lineNumber, "face vertex has too many components");

		const std::string position = token.substr(0, firstSlash);
		if (position.empty())
			throwLineError(filename, lineNumber, "face vertex is missing a position index");

		const std::string texcoord = firstSlash == std::string::npos
			? ""
			: token.substr(firstSlash + 1, secondSlash - firstSlash - 1);
		const std::string normal = secondSlash == std::string::npos
			? ""
			: token.substr(secondSlash + 1);

		OBJIndex index;
		index.position = resolveIndex(parseInteger(position, filename, lineNumber, "position"),
									  data.positions.size(), filename, lineNumber, "position");
		index.texcoord = parseOptionalIndex(texcoord, data.texcoords.size(), filename, lineNumber, "texcoord");
		index.normal = parseOptionalIndex(normal, data.normals.size(), filename, lineNumber, "normal");
		return index;
	}
}

OBJData OBJParser::parse(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file)
		throw std::runtime_error("cannot open OBJ file '" + filename + "'");

	OBJData data;
	std::string currentMaterial;
	std::string line;
	std::size_t lineNumber = 0;

	while (std::getline(file, line))
	{
		++lineNumber;
		const std::size_t comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment);

		std::istringstream input(line);
		std::string command;
		if (!(input >> command))
			continue;

		if (command == "v")
		{
			std::string x;
			std::string y;
			std::string z;
			if (!(input >> x >> y >> z))
				throwLineError(filename, lineNumber, "vertex requires three coordinates");
			data.positions.emplace_back(parseFloat(x, filename, lineNumber, "vertex coordinate"),
										 parseFloat(y, filename, lineNumber, "vertex coordinate"),
										 parseFloat(z, filename, lineNumber, "vertex coordinate"));
		}
		else if (command == "vt")
		{
			std::string u;
			std::string v;
			if (!(input >> u >> v))
				throwLineError(filename, lineNumber, "texture coordinate requires two values");
			data.texcoords.emplace_back(parseFloat(u, filename, lineNumber, "texture coordinate"),
										parseFloat(v, filename, lineNumber, "texture coordinate"));
		}
		else if (command == "vn")
		{
			std::string x;
			std::string y;
			std::string z;
			if (!(input >> x >> y >> z))
				throwLineError(filename, lineNumber, "normal requires three coordinates");
			data.normals.emplace_back(parseFloat(x, filename, lineNumber, "normal coordinate"),
									   parseFloat(y, filename, lineNumber, "normal coordinate"),
									   parseFloat(z, filename, lineNumber, "normal coordinate"));
		}
		else if (command == "mtllib")
		{
			std::string library;
			std::getline(input, library);
			data.materialLibrary = trim(library);
			if (data.materialLibrary.empty())
				throwLineError(filename, lineNumber, "mtllib requires a filename");
		}
		else if (command == "usemtl")
		{
			std::getline(input, currentMaterial);
			currentMaterial = trim(currentMaterial);
			if (currentMaterial.empty())
				throwLineError(filename, lineNumber, "usemtl requires a material name");
		}
		else if (command == "f")
		{
			OBJFace face;
			face.materialName = currentMaterial;
			std::string token;
			while (input >> token)
				face.vertices.push_back(parseFaceVertex(token, data, filename, lineNumber));

			if (face.vertices.size() < 3)
				throwLineError(filename, lineNumber, "face requires at least three vertices");
			data.faces.push_back(std::move(face));
		}
	}

	return data;
}

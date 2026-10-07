#include "MTLParser.hpp"

#include <fstream>
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
					 const std::string& property)
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
			throwLineError(filename, lineNumber, "invalid value for " + property + " '" + text + "'");
		}
	}

	int parseInteger(const std::string& text,
					 const std::string& filename,
					 std::size_t lineNumber,
					 const std::string& property)
	{
		std::size_t consumed = 0;
		try
		{
			const int value = std::stoi(text, &consumed);
			if (consumed != text.size())
				throw std::invalid_argument("trailing characters");
			return value;
		}
		catch (const std::exception&)
		{
			throwLineError(filename, lineNumber, "invalid value for " + property + " '" + text + "'");
		}
	}

	void requireMaterial(bool hasMaterial,
						 const std::string& filename,
						 std::size_t lineNumber)
	{
		if (!hasMaterial)
			throwLineError(filename, lineNumber, "property appears before any newmtl declaration");
	}

	void requireNoExtraValues(std::istringstream& input,
							  const std::string& filename,
							  std::size_t lineNumber,
							  const std::string& property)
	{
		std::string extra;
		if (input >> extra)
			throwLineError(filename, lineNumber, property + " has too many values");
	}

	float parseSingleFloat(std::istringstream& input,
							  const std::string& filename,
							  std::size_t lineNumber,
							  const std::string& property)
	{
		std::string value;
		if (!(input >> value))
			throwLineError(filename, lineNumber, property + " requires one value");

		const float result = parseFloat(value, filename, lineNumber, property);
		requireNoExtraValues(input, filename, lineNumber, property);
		return result;
	}

	Vec3 parseColor(std::istringstream& input,
					const std::string& filename,
					std::size_t lineNumber,
					const std::string& property)
	{
		std::string x;
		std::string y;
		std::string z;
		if (!(input >> x >> y >> z))
			throwLineError(filename, lineNumber, property + " requires three values");

		const Vec3 result{
			parseFloat(x, filename, lineNumber, property),
			parseFloat(y, filename, lineNumber, property),
			parseFloat(z, filename, lineNumber, property)};
		requireNoExtraValues(input, filename, lineNumber, property);
		return result;
	}
}

MTLData MTLParser::parse(const std::string& filename)
{
	std::ifstream file(filename);
	if (!file)
		throw std::runtime_error("cannot open MTL file '" + filename + "'");

	MTLData data;
	Material currentMaterial;
	bool hasMaterial = false;
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

		if (command == "newmtl")
		{
			std::string name;
			std::getline(input, name);
			name = trim(name);
			if (name.empty())
				throwLineError(filename, lineNumber, "newmtl requires a material name");

			if (hasMaterial)
				data.materials[currentMaterial.name] = currentMaterial;

			currentMaterial = Material{};
			currentMaterial.name = name;
			hasMaterial = true;
		}
		else if (command == "Ns" || command == "Ni" || command == "d")
		{
			requireMaterial(hasMaterial, filename, lineNumber);
			const float value = parseSingleFloat(input, filename, lineNumber, command);
			if (command == "Ns")
				currentMaterial.Ns = value;
			else if (command == "Ni")
				currentMaterial.Ni = value;
			else
				currentMaterial.d = value;
		}
		else if (command == "Ka" || command == "Kd" || command == "Ks")
		{
			requireMaterial(hasMaterial, filename, lineNumber);
			const Vec3 value = parseColor(input, filename, lineNumber, command);
			if (command == "Ka")
				currentMaterial.Ka = value;
			else if (command == "Kd")
				currentMaterial.Kd = value;
			else
				currentMaterial.Ks = value;
		}
		else if (command == "Tr")
		{
			requireMaterial(hasMaterial, filename, lineNumber);
			currentMaterial.d = 1.0f - parseSingleFloat(input, filename, lineNumber, command);
		}
		else if (command == "illum")
		{
			requireMaterial(hasMaterial, filename, lineNumber);
			std::string value;
			if (!(input >> value))
				throwLineError(filename, lineNumber, "illum requires one value");
			currentMaterial.illum = parseInteger(value, filename, lineNumber, command);
			requireNoExtraValues(input, filename, lineNumber, command);
		}
		else if (command == "map_Kd")
		{
			requireMaterial(hasMaterial, filename, lineNumber);
			std::string texture;
			std::getline(input, texture);
			texture = trim(texture);
			if (texture.empty())
				throwLineError(filename, lineNumber, "map_Kd requires a texture filename");
			currentMaterial.diffuseTexture = texture;
		}
	}

	if (hasMaterial)
		data.materials[currentMaterial.name] = std::move(currentMaterial);

	return data;
}
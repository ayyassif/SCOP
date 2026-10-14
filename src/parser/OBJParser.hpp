#ifndef OBJPARSER_HPP
#define OBJPARSER_HPP

#include "OBJTypes.hpp"

#include <string>

class OBJParser
{
public:
	OBJData parse(const std::string& filename);
};

#endif

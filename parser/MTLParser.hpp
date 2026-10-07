#ifndef MTLPARSER_HPP
#define MTLPARSER_HPP

#include "MTLTypes.hpp"

#include <string>

class MTLParser
{
public:
	MTLData parse(const std::string& filename);
};

#endif
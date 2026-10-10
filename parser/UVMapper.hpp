 #ifndef UVMAPPER_HPP
 #define UVMAPPER_HPP
 
#include "OBJTypes.hpp"

using Charts = std::vector<std::vector<int>>;

class UVMapper
{
public:
	void generate(OBJData& data);
};

 #endif

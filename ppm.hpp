#ifndef PPM_HPP
#define PPM_HPP

#include <string>
#include <vector>
#include <fstream>
#include <limits>
#include <stdexcept>

class PPM
{
public:
    PPM();

    void load(const std::string& path);

    int  getWidth() const;
    int  getHeight() const;

    const std::vector<unsigned char>& getPixels() const;
private:
    int width;
    int height;
    int maxColor;
    std::vector<unsigned char> pixels;
    

    std::string readToken(std::ifstream& file);
    void        skipSpaceAndComments(std::ifstream& file);
    int         parseInteger(const std::string& token, const std::string& field);
};

#endif
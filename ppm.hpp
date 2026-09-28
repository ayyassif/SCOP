#ifndef PPM_HPP
#define PPM_HPP

#include <string>
#include <vector>
#include <fstream>
// #include <stdexcept>
#include <limits>

class PPM
{
public:
    PPM();

    void load(const std::string& path);

    int getWidth() const;
    int getHeight() const;
    const std::vector<unsigned char>& getPixels() const;

private:
    int width;
    int height;
    int maxColor;
    std::vector<unsigned char> pixels;

    void skipSpaceAndComments(std::ifstream& file);
    std::string readToken(std::ifstream& file);
};

#endif
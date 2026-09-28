#include "ppm.hpp"

PPM::PPM() : width(0), height(0), maxColor(0)
{   
}

void PPM::load(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file || readToken(file) != "P6")
        return ; // throw error
    
    width = std::stoi(readToken(file)); // error needs to be catched
    height = std::stoi(readToken(file));
    maxColor = std::stoi(readToken(file));

    if (width <= 0 || height <= 0 || maxColor != 255)
        return ; // throw error

    const size_t pixelSize =
        static_cast<size_t>(width) *
        static_cast<size_t>(height) *
        3;
    
    pixels.resize(pixelSize);

    file.read(
        reinterpret_cast<char *>(pixels.data()),
        static_cast<std::streamsize>(pixelSize)
    );
    if (!file)
    {
        pixels.clear();
        return ;// throw error
    }
}

int PPM::getWidth() const
{
    return width;
}

int PPM::getHeight() const
{
    return height;
}

const std::vector<unsigned char>& PPM::getPixels() const
{
    return pixels;
}

void PPM::skipSpaceAndComments(std::ifstream& file)
{
    char c;

    while (file.get(c))
    {
        if (c == '#')
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        else if (!std::isspace(static_cast<unsigned char>(c)))
        {
            file.unget();
            return ;
        }
    }
}

std::string PPM::readToken(std::ifstream& file)
{
    skipSpaceAndComments(file);

    std::string token;
    char        c;

    while (file.get(c))
    {
        if (std::isspace(static_cast<unsigned char>(c)))
            break;
        if (c == '#')
        {
            file.unget();
            break;
        }

        token += c;
    }

    return token;
}
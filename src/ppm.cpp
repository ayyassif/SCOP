#include "ppm.hpp"

#include <cctype>
#include <utility>

PPM::PPM() : width(0), height(0), maxColor(0), pixels() {}

int PPM::parseInteger(const std::string& token, const std::string& field)
{
    if (token.empty())
        throw std::invalid_argument("PPM: missing " + field);

    std::size_t parsedCharacters = 0;
    long long value;

    try
    {
        value = std::stoll(token, &parsedCharacters);
    }
    catch (const std::exception&)
    {
        throw std::invalid_argument("PPM: invalid " + field);
    }

    if (parsedCharacters != token.size())
        throw std::invalid_argument("PPM: invalid " + field);
    if (value < std::numeric_limits<int>::min() ||
        value > std::numeric_limits<int>::max())
        throw std::out_of_range("PPM: " + field + " is out of range");

    return static_cast<int>(value);
}

void PPM::load(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);

    if (!file)
        throw std::runtime_error("PPM: cannot open file '" + path + "'");
    if (readToken(file) != "P6")
        throw std::invalid_argument("PPM: expected P6 format");
    
    width = parseInteger(readToken(file), "width");
    height = parseInteger(readToken(file), "height");
    maxColor = parseInteger(readToken(file), "maximum color value");

    if (width <= 0 || height <= 0 || maxColor != 255)
        throw std::invalid_argument("PPM: invalid image dimensions or maximum color value");

    const size_t pixelCount =
        static_cast<size_t>(width) * static_cast<size_t>(height);
    if (pixelCount > std::numeric_limits<size_t>::max() / 3)
        throw std::length_error("PPM: image is too large");

    const size_t pixelSize = pixelCount * 3;

    if (pixelSize > static_cast<size_t>(std::numeric_limits<std::streamsize>::max()))
        throw std::length_error("PPM: image is too large for the input stream");
    
    pixels.resize(pixelSize);

    file.read(
        reinterpret_cast<char *>(pixels.data()),
        static_cast<std::streamsize>(pixelSize)
    );
    if (!file)
    {
        pixels.clear();
        throw std::runtime_error("PPM: pixel data is incomplete");
    }

    // flips the image
    const size_t rowSize = static_cast<size_t>(width) * 3;
    for (int row = 0; row < height / 2; ++row)
    {
        const size_t top = static_cast<size_t>(row) * rowSize;
        const size_t bottom =
            static_cast<size_t>(height - row - 1) * rowSize;

        for (size_t column = 0; column < rowSize; ++column)
            std::swap(pixels[top + column], pixels[bottom + column]);
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
#include "PALFile.h"

SCRANTIC::PALFile::PALFile(std::string name, std::vector<u_int8_t> &data) : BaseFile(name)
{
    std::vector<u_int8_t>::iterator it = data.begin();

    std::string tmp = read_const_string(it, 4);
    if (tmp != "PAL:")
    {
        std::cerr << filename << ": \"PAL:\" expected; got " << tmp << std::endl;
        return;
    }

    u_read_le(it, vgaSize);
    u_read_le(it, magic);

    tmp = read_const_string(it, 4);
    if (tmp != "VGA:")
    {
        std::cerr << filename << ": \"VGA:\" expected; got " << tmp << std::endl;
        return;
    }

    u_read_le(it, palCount);
    /*if (palCount > 255)
    {
        std::cerr << filename << ": Palette count too large! " << palCount << std::endl;
        palCount = 256;
    }*/

    u_int8_t r,g,b;
    SDL_Color color;
    color.a = 0;

    for (u_int32_t i = 0; i < palCount; i++)
    {
        u_read_le(it, r);
        u_read_le(it, g);
        u_read_le(it, b);
        //palette.push_back(std::make_tuple< u_int8_t, u_int8_t, u_int8_t >(r*4, g*4, b*4));
        color.r = r*4;
        color.g = g*4;
        color.b = b*4;
        palette.push_back(color);
        color.a = 255;
    }
}

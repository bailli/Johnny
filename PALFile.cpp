#include "PALFile.h"

SCRANTIC::PALFile::PALFile(const std::string &name, v8 &data)
    : BaseFile(name) {

    v8::iterator it = data.begin();

    assertString(it, "PAL:");

    readUintLE(it, vgaSize);
    readUintLE(it, magic);

    assertString(it, "VGA:");

    readUintLE(it, palCount);
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
        readUintLE(it, r);
        readUintLE(it, g);
        readUintLE(it, b);
        //palette.push_back(std::make_tuple< u_int8_t, u_int8_t, u_int8_t >(r*4, g*4, b*4));
        color.r = r*4;
        color.g = g*4;
        color.b = b*4;
        palette.push_back(color);
        color.a = 255;
    }
}

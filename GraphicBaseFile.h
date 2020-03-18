#ifndef GRAPHICBASEFILE_H
#define GRAPHICBASEFILE_H

#include "types.h"

#include <iostream>
#include <sstream>
#include <fstream>

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace SCRANTIC {

struct BitmapFileHeader {
    u16 bfType;
    u32 bfSize;
    u32 bfReserved;
    u32 bfOffsetBits;
};

struct BitmapInfoHeader {
    u32 biSize;
    i32 biWidth;
    i32 biHeight;
    u16 biPlanes;
    u16 biBitCount;
    u32 biCompression;
    u32 biSizeImage;
    i32 biXPelsPerMeter;
    i32 biYPelsPerMeter;
    u32 biClrUsed;
    u32 biClrImportant;
    u32 biRedMask;
    u32 biGreenMask;
    u32 biBlueMask;
    u32 biAlphaMask;
    u32 biColourSpace;
    u32 biCsEndpoints[9];
    u32 biRedGamma;
    u32 biGreenGamma;
    u32 biBlueGamma;
};

class GraphicBaseFile {
private:
    i8 matchSdlColorToPaletteNumber(SDL_Color &color);

protected:
    SDL_Surface* createSdlSurface(v8 &data, u16 width, u16 height, size_t offset = 0);
    v8 createRGBABitmapData(v8 &data, u16 width, u16 height, size_t offset = 0);
    v8 readRGBABitmapData(const std::string &filename, u16 &width, u16 &height);

    SDL_Color defaultPalette[256];

public:

public:

public:
    GraphicBaseFile();
    ~GraphicBaseFile() {};
};

}

#endif // GRAPHICBASEFILE_H

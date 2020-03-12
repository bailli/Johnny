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

class GraphicBaseFile {
private:
    i8 matchSdlColorToPaletteNumber(SDL_Color &color);

protected:
    SDL_Surface* createSdlSurface(v8 &data, u16 width, u16 height, size_t offset = 0);
    v8 convertScrToRgbData(const v8 &data);
    v8 convertRgbDataToScr(std::ifstream &in, u32 pixelCount);

    SDL_Color defaultPalette[256];

public:

public:

public:
    GraphicBaseFile();
    ~GraphicBaseFile() {};
};

}

#endif // GRAPHICBASEFILE_H

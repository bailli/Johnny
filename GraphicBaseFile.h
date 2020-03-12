#ifndef GRAPHICBASEFILE_H
#define GRAPHICBASEFILE_H

#include "types.h"

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace SCRANTIC {

class GraphicBaseFile {
protected:
    SDL_Surface* createSdlSurface(v8 &data, u16 width, u16 height, size_t offset = 0);
    v8 convertScrToRgbData(const v8 &data);
    i8 matchSdlColorToPaletteNumber(SDL_Color &color);

    SDL_Color defaultPalette[256];

public:
    GraphicBaseFile();
    ~GraphicBaseFile() {};
};

}

#endif // GRAPHICBASEFILE_H

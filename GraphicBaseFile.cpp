#include "GraphicBaseFile.h"

#include "BaseFile.h"

SCRANTIC::GraphicBaseFile::GraphicBaseFile() {
    defaultPalette[0]  = { 168,   0, 168,   0};
    defaultPalette[1]  = {   0,   0, 168, 255};
    defaultPalette[2]  = {   0, 168,   0, 255};
    defaultPalette[3]  = {   0, 168, 168, 255};
    defaultPalette[4]  = { 168,   0,   0, 255};
    defaultPalette[5]  = {   0,   0,   0, 255};
    defaultPalette[6]  = { 168, 168,   0, 255};
    defaultPalette[7]  = { 212, 212, 212, 255};
    defaultPalette[8]  = { 128, 128, 128, 255};
    defaultPalette[9]  = {   0,   0, 255, 255};
    defaultPalette[10] = {   0, 255,   0, 255};
    defaultPalette[11] = {   0, 255, 255, 255};
    defaultPalette[12] = { 255,   0,   0, 255};
    defaultPalette[13] = { 255,   0, 255, 255};
    defaultPalette[14] = { 255, 255,   0, 255};
    defaultPalette[15] = { 255, 255, 255, 255};
}

v8 SCRANTIC::GraphicBaseFile::convertScrToRgbData(const v8 &data) {
    v8 bmpData;
    SDL_Color color;

    for (size_t i = 0; i < data.size(); ++i) {
        color = defaultPalette[data[i] >> 4];
        bmpData.push_back(color.r);
        bmpData.push_back(color.g);
        bmpData.push_back(color.b);

        color = defaultPalette[data[i] & 0xF];
        bmpData.push_back(color.r);
        bmpData.push_back(color.g);
        bmpData.push_back(color.b);
    }

    return bmpData;
}

SDL_Surface* SCRANTIC::GraphicBaseFile::createSdlSurface(v8 &data, u16 width, u16 height, size_t offset) {
    SDL_Surface *surface = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    SDL_SetPaletteColors(surface->format->palette, defaultPalette, 0, 256);

    size_t z = offset;
    bool high = false;
    u8 idx;

    unsigned char *p = (unsigned char*)surface->pixels;

    for (int y  = 0; y < surface->h; ++y) {
        for (int x = 0; x < surface->w; ++x) {
            if (high) {
                high = false;
                idx = data[z] & 0xF;
                z++;
            } else {
                high = true;
                idx = data[z] >> 4;
            }
            p[y * surface->w + x] = idx;
        }
    }

    return surface;
}

v8 SCRANTIC::GraphicBaseFile::convertRgbDataToScr(std::ifstream &in, u32 pixelCount) {
    SDL_Color color;
    i8 palIndex;
    u8 r, g, b;
    u8 byte;
    bool high = false;
    v8 scrData;

    for (u32 i = 0; i < pixelCount; i += 3) {
        BaseFile::readUintLE(&in, r);
        BaseFile::readUintLE(&in, g);
        BaseFile::readUintLE(&in, b);
        color = { r, g, b, 0 };

        palIndex = matchSdlColorToPaletteNumber(color);
        if (palIndex != -1) {
            if (!high) {
                byte = palIndex << 4;
                high = true;
            } else {
                byte |= palIndex;
                scrData.push_back(byte);
                high = false;
            }
        }
    }
    return scrData;
}


i8 SCRANTIC::GraphicBaseFile::matchSdlColorToPaletteNumber(SDL_Color& color) {
    for (u8 i = 0; i < 16; ++i) {
        if ((defaultPalette[i].r == color.r)
            && (defaultPalette[i].g == color.g)
            && (defaultPalette[i].b == color.b)) {
            //&& (defaultPalette[i].a == color.a)) {
            return i;
        }
    }
    return -1;
}

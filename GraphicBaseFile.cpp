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

i8 SCRANTIC::GraphicBaseFile::matchSdlColorToPaletteNumber(SDL_Color& color) {
    for (u8 i = 0; i < 16; ++i) {
        if ((defaultPalette[i].r == color.r)
                && (defaultPalette[i].g == color.g)
                && (defaultPalette[i].b == color.b)
                && (defaultPalette[i].a == color.a)) {
            return i;
        }
    }
    return -1;
}

v8 SCRANTIC::GraphicBaseFile::createRGBABitmapData(v8 &data, u16 width, u16 height, size_t offset) {
    BitmapFileHeader header;
    BitmapInfoHeader info;

    header.bfType = 0x4D42;
    header.bfSize = width * height * 4 + 124;
    header.bfReserved = 0;
    header.bfOffsetBits = 124;

    info.biSize = sizeof(BitmapInfoHeader);
    info.biWidth = width;
    info.biHeight = height;
    info.biPlanes = 1;
    info.biBitCount = 32;
    info.biCompression = 3;//0;
    info.biSizeImage = width * height;
    info.biXPelsPerMeter = 0xB13;
    info.biYPelsPerMeter = 0xB13;
    info.biClrUsed = 0x0; //16;
    info.biClrImportant = 0;
    info.biRedMask = 0xFF;//0;
    info.biGreenMask = 0xFF00;//0;
    info.biBlueMask = 0xFF0000;//0;
    info.biAlphaMask = 0xFF000000;//0;
    info.biColourSpace = 0;
    info.biCsEndpoints[0] = 0;
    info.biCsEndpoints[1] = 0;
    info.biCsEndpoints[2] = 0;
    info.biCsEndpoints[3] = 0;
    info.biCsEndpoints[4] = 0;
    info.biCsEndpoints[5] = 0;
    info.biCsEndpoints[6] = 0;
    info.biCsEndpoints[7] = 0;
    info.biCsEndpoints[8] = 0;
    info.biRedGamma = 0;
    info.biGreenGamma = 0;
    info.biBlueGamma = 0;

    //16 * SDL_Color (4x u8 => 64 bytes)
    // u16 padding
    //image data: width*height

    v8 bitmapData;
    BaseFile::writeUintLE(bitmapData, header.bfType);
    BaseFile::writeUintLE(bitmapData, header.bfSize);
    BaseFile::writeUintLE(bitmapData, header.bfReserved);
    BaseFile::writeUintLE(bitmapData, header.bfOffsetBits);

    BaseFile::writeUintLE(bitmapData, info.biSize);
    BaseFile::writeUintLE(bitmapData, info.biWidth);
    BaseFile::writeUintLE(bitmapData, info.biHeight);
    BaseFile::writeUintLE(bitmapData, info.biPlanes);
    BaseFile::writeUintLE(bitmapData, info.biBitCount);
    BaseFile::writeUintLE(bitmapData, info.biCompression);
    BaseFile::writeUintLE(bitmapData, info.biSizeImage);
    BaseFile::writeUintLE(bitmapData, info.biXPelsPerMeter);
    BaseFile::writeUintLE(bitmapData, info.biYPelsPerMeter);
    BaseFile::writeUintLE(bitmapData, info.biClrUsed);
    BaseFile::writeUintLE(bitmapData, info.biClrImportant);
    BaseFile::writeUintLE(bitmapData, info.biRedMask);
    BaseFile::writeUintLE(bitmapData, info.biGreenMask);
    BaseFile::writeUintLE(bitmapData, info.biBlueMask);
    BaseFile::writeUintLE(bitmapData, info.biAlphaMask);

    u32 fillHeader = 0;
    for (int i = 0; i < 13; ++i) {
        BaseFile::writeUintLE(bitmapData, fillHeader);
    }

    u16 pad = 0;
    BaseFile::writeUintLE(bitmapData, pad);

    SDL_Color color;
    for (i32 i = height - 1; i >= 0; --i) {
        for (u16 j = 0; j < width/2; ++j) {
            size_t pos = offset + i*width/2 + j;
            color = defaultPalette[data[pos] >> 4];
            bitmapData.push_back(color.r);
            bitmapData.push_back(color.g);
            bitmapData.push_back(color.b);
            bitmapData.push_back(color.a);

            color = defaultPalette[data[pos] & 0xF];
            bitmapData.push_back(color.r);
            bitmapData.push_back(color.g);
            bitmapData.push_back(color.b);
            bitmapData.push_back(color.a);
        }
    }

    return bitmapData;
}


v8 SCRANTIC::GraphicBaseFile::readRGBABitmapData(const std::string &filename, u16 &width, u16 &height) {
    std::ifstream in;
    in.open(filename, std::ios::binary | std::ios::in);
    in.unsetf(std::ios::skipws);

    u8 byte;
    v8 data;

    while (in.read((char*)&byte, 1)) {
        data.push_back(byte);
    }

    in.close();

    v8 scrData;

    if (data.size() < 122) {
        std::cout << "ERROR: only 32bit RGBA bitmaps are supported" << std::endl;
        width = 0;
        height = 0;
        return scrData;
    }

    v8::iterator it = data.begin();

    BitmapFileHeader header;
    BitmapInfoHeader info;

    BaseFile::readUintLE(it, header.bfType);
    BaseFile::readUintLE(it, header.bfSize);
    BaseFile::readUintLE(it, header.bfReserved);
    BaseFile::readUintLE(it, header.bfOffsetBits);

    BaseFile::readUintLE(it, info.biSize);
    BaseFile::readUintLE(it, info.biWidth);
    BaseFile::readUintLE(it, info.biHeight);
    BaseFile::readUintLE(it, info.biPlanes);
    BaseFile::readUintLE(it, info.biBitCount);
    BaseFile::readUintLE(it, info.biCompression);
    BaseFile::readUintLE(it, info.biSizeImage);
    BaseFile::readUintLE(it, info.biXPelsPerMeter);
    BaseFile::readUintLE(it, info.biYPelsPerMeter);
    BaseFile::readUintLE(it, info.biClrUsed);
    BaseFile::readUintLE(it, info.biClrImportant);
    BaseFile::readUintLE(it, info.biRedMask);
    BaseFile::readUintLE(it, info.biGreenMask);
    BaseFile::readUintLE(it, info.biBlueMask);
    BaseFile::readUintLE(it, info.biAlphaMask);

    if ((header.bfType != 0x4D42) || (info.biBitCount != 32) || (info.biCompression != 3) || (info.biRedMask != 0xFF)
        || (info.biGreenMask != 0xFF00) || (info.biBlueMask != 0xFF0000) || (info.biAlphaMask != 0xFF000000)) {
        std::cout << "ERROR: only 32bit RGBA bitmaps are supported" << std::endl;
        width = 0;
        height = 0;
        return scrData;
    }

    width = info.biWidth;
    height = info.biHeight;

    SDL_Color color;
    i8 palIndex;
    bool high = false;

    for (i32 i = height - 1; i >= 0; --i) {
        for (u16 j = 0; j < width; ++j) {
            size_t pos = header.bfOffsetBits + i*width*4 + j*4;
            color = { data[pos], data[pos+1], data[pos+2], data[pos+3] };

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
    }

    return scrData;
}

#include "SCRFile.h"

SCRANTIC::SCRFile::SCRFile(const std::string &name, v8 &data) :
    CompressedBaseFile(name),
    image(NULL),
    texture(NULL) {

    v8::iterator it = data.begin();

    assertString(it, "SCR:");

    readUintLE(it, dimBinSize);
    readUintLE(it, magic);

    assertString(it, "DIM:");

    readUintLE(it, dimSize);
    readUintLE(it, width);
    readUintLE(it, height);

    assertString(it, "BIN:");

    if (!handleDecompression(data, it, uncompressedData)) {
        return;
    }

    image = createSdlSurface(uncompressedData, width, height);
}

v8 SCRANTIC::SCRFile::repackIntoResource() {

    std::string strings[3] = { "SCR:", "DIM:", "BIN:" };

    magic = 0x8000;

    compressionFlag = 2;
    v8 compressedData = LZCCompress(uncompressedData);
    uncompressedSize = uncompressedData.size();
    compressedSize = compressedData.size() + 5;

    dimSize = 4;
    dimBinSize = dimSize + compressedSize + 16;

    v8 rawData(strings[0].begin(), strings[0].end());
    BaseFile::writeUintLE(rawData, dimBinSize);
    BaseFile::writeUintLE(rawData, magic);
    std::copy(strings[1].begin(), strings[1].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, dimSize);
    BaseFile::writeUintLE(rawData, width);
    BaseFile::writeUintLE(rawData, height);
    std::copy(strings[2].begin(), strings[2].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, compressedSize);
    BaseFile::writeUintLE(rawData, compressionFlag);
    BaseFile::writeUintLE(rawData, uncompressedSize);
    std::copy(compressedData.begin(), compressedData.end(), std::back_inserter(rawData));

    compressedSize -= 5;

    return rawData;
}

SCRANTIC::SCRFile::SCRFile(const std::string &bmpFilename)
    : CompressedBaseFile(bmpFilename),
      image(NULL),
      texture(NULL) {

    std::string actualFilename = bmpFilename.substr(0, bmpFilename.rfind('.')) + ".BMP";

    uncompressedData = readRGBABitmapData(actualFilename, width, height);

    image = createSdlSurface(uncompressedData, width, height);
}

void SCRANTIC::SCRFile::saveFile(const std::string &path) {
    v8 bmpFile = createRGBABitmapData(uncompressedData, width, height);

    std::string newFilename = filename.substr(0, filename.rfind('.')) + ".BMP";

    SCRANTIC::BaseFile::writeFile(bmpFile, newFilename, path);
}

SCRANTIC::SCRFile::~SCRFile() {
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(image);
}

void SCRANTIC::SCRFile::setPalette(SDL_Color color[], u16 count) {
    if (image == NULL) {
        return;
    }

    SDL_SetPaletteColors(image->format->palette, color, 0, 256);
    SDL_DestroyTexture(texture);
    texture = NULL;
}

SDL_Texture *SCRANTIC::SCRFile::getImage(SDL_Renderer *renderer, SDL_Rect &rect) {
    if (image == NULL) {
        return NULL;
    }

    if (texture == NULL) {
        texture = SDL_CreateTextureFromSurface(renderer, image);
        if (texture == NULL) {
            std::cerr << filename << ": Error creating SDL_Texture" << std::endl;
            return NULL;
        }
    }

    rect = { 0, 0, image->w, image->h };

    return texture;
}

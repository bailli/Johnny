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

SCRANTIC::SCRFile::SCRFile(const std::string &ppmFilename)
    : CompressedBaseFile(ppmFilename),
      image(NULL),
      texture(NULL) {

    uncompressedData = readRGBABitmapData(filename, width, height);
    filename = ppmFilename.substr(0, ppmFilename.rfind('.')) + ".SCR";


//     readString(&in, 0, '\n'); /* header: P6 */
//     std::string dimension = readString(&in, 0, '\n');
//     readString(&in, 0, '\n'); /* 255 */
//
//     width = std::stoi(dimension.substr(0, dimension.find(' ')));
//     height = std::stoi(dimension.substr(dimension.find(' ') + 1));
//
//     u32 ppmSize = width * height * 3;
//     uncompressedData = convertRgbDataToScr(in, ppmSize);
//
//     in.close();

    image = createSdlSurface(uncompressedData, width, height);
}

void SCRANTIC::SCRFile::saveFile(const std::string &path) {
//     std::string header = "P6\n" + hexToString(width, std::dec) + " " + hexToString(height, std::dec) + "\n255\n";
//     v8 ppmFile(header.begin(), header.end());
//
//     v8 bitmapData = convertScrToRgbData(uncompressedData);
//
//     std::copy(bitmapData.begin(), bitmapData.end(), std::back_inserter(ppmFile));

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

    rect.w = image->w;
    rect.h = image->h;

    return texture;
}

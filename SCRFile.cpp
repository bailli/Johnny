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

    //saveFile(uncompressedData, filename, "res/SCR/");

    image = createSdlSurface(uncompressedData, width, height);
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

#ifndef SCRFILE_H
#define SCRFILE_H

#include "CompressedBaseFile.h"
#include "GraphicBaseFile.h"

namespace SCRANTIC {

class SCRFile : public CompressedBaseFile, public GraphicBaseFile {
protected:
    u16 dimBinSize; //
    u16 magic;  //0x8000
    u32 dimSize;
    u16 imageCount;
    u16 width;
    u16 height;
    v8 uncompressedData;
    SDL_Surface *image;
    SDL_Texture *texture;

public:
    SCRFile(const std::string &name, v8 &data);
    explicit SCRFile(const std::string &ppmFilename);
    ~SCRFile();

    void saveFile(std::string path = "");

    SDL_Texture *getImage(SDL_Renderer *renderer, SDL_Rect &rect);
    void setPalette(SDL_Color color[], u16 count);
};

}

#endif // SCRFILE_H


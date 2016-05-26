#ifndef SCRFILE_H
#define SCRFILE_H

#include "BaseFile.h"

namespace SCRANTIC {

class SCRFile : public BaseFile
{
protected:
    u_int16_t dimBinSize; //
    u_int16_t magic;  //0x8000
    u_int32_t dimSize;
    u_int16_t imageCount;
    u_int16_t width;
    u_int16_t height;
    u_int32_t binSize;
    u_int8_t compressionFlag;
    u_int32_t uncompressedSize;
    std::vector<u_int8_t> uncompressedData;
    SDL_Surface *image;
    SDL_Texture *texture;

public:
    SCRFile(std::string name, std::vector<u_int8_t> &data);
    ~SCRFile();
    SDL_Texture *getImage(SDL_Renderer *renderer, SDL_Rect &rect);
    void setPalette(SDL_Color color[], u_int16_t count);
};

}

#endif // SCRFILE_H


#ifndef BMPFILE_H
#define BMPFILE_H

#include "BaseFile.h"

namespace SCRANTIC {

class BMPFile : public BaseFile
{
protected:
    u_int16_t infBinSize;
    u_int16_t magic;  //0x8000
    u_int32_t infSize;
    u_int16_t imageCount;
    std::vector<u_int16_t> widthList;
    std::vector<u_int16_t> heightList;
    u_int32_t binSize;
    u_int8_t compressionFlag;
    u_int32_t uncompressedSize;
    std::vector<u_int8_t> uncompressedData;
    std::vector<SDL_Surface *> imageList;
    std::vector<SDL_Rect> imageRect;
    SDL_Surface *overview;
    SDL_Texture *ovTexture;
    void createOverview();

public:
    BMPFile(std::string name, std::vector<u_int8_t> &data);
    ~BMPFile();
    SDL_Texture *getImage(SDL_Renderer *renderer, u_int16_t num, SDL_Rect &rect);
    size_t getImageCount() { return imageList.size(); }
    SDL_Texture *getOverviewImage(SDL_Renderer *renderer, SDL_Rect &rect);
    void setPalette(SDL_Color color[], u_int16_t count);
    SDL_Rect getRect(u_int16_t num);

};

}

#endif // BMPFILE_H


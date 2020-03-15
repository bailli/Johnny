#ifndef BMPFILE_H
#define BMPFILE_H

#include "CompressedBaseFile.h"
#include "GraphicBaseFile.h"

namespace SCRANTIC {

class BMPFile : public CompressedBaseFile, public GraphicBaseFile {
protected:
    u16 infBinSize;
    u16 magic;  //0x8000
    u32 infSize;
    u16 imageCount;
    v16 widthList;
    v16 heightList;
    v8 uncompressedData;
    std::vector<SDL_Surface *> imageList;
    std::vector<SDL_Rect> imageRect;
    SDL_Surface *overview;
    SDL_Texture *ovTexture;

    void createOverview();

public:
    BMPFile(const std::string &name, v8 &data);
    ~BMPFile();

    void saveFile(const std::string &path);
    v8 repackIntoResource() override;

    SDL_Texture *getImage(SDL_Renderer *renderer, u16 num, SDL_Rect &rect);
    SDL_Texture *getOverviewImage(SDL_Renderer *renderer, SDL_Rect &rect);
    void setPalette(SDL_Color color[], u16 count);
    SDL_Rect getRect(u16 num);

    size_t getImageCount() {
        return imageList.size();
    }
};

}

#endif // BMPFILE_H


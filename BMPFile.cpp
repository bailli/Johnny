#include "BMPFile.h"

SCRANTIC::BMPFile::BMPFile(const std::string &name, v8 &data)
    : CompressedBaseFile(name),
      GraphicBaseFile(),
      overview(NULL),
      ovTexture(NULL) {

    v8::iterator it = data.begin();

    assertString(it, "BMP:");

    readUintLE(it, infBinSize);
    readUintLE(it, magic);

    assertString(it, "INF:");

    readUintLE(it, infSize);
    readUintLE(it, imageCount);

    u16 word;
    for (int i = 0; i < imageCount; ++i) {
        readUintLE(it, word);
        widthList.push_back(word);
    }

    for (int i = 0; i < imageCount; ++i) {
        readUintLE(it, word);
        heightList.push_back(word);
    }

    assertString(it, "BIN:");

    if (!handleDecompression(data, it, uncompressedData)) {
        return;
    }

    //saveFile(uncompressedData, filename, "res/BMP/");

    size_t z = 0;
    for (u16 i = 0; i < imageCount; ++i) {
        SDL_Surface *image = createSdlSurface(uncompressedData, widthList.at(i), heightList.at(i), z);
        z += widthList.at(i) * heightList.at(i) / 2;
        imageList.push_back(image);
    }

    createOverview();
}

SCRANTIC::BMPFile::~BMPFile() {
    for (auto i = std::begin(imageList); i != std::end(imageList); ++i) {
        SDL_FreeSurface(*i);
    }

    SDL_FreeSurface(overview);
    SDL_DestroyTexture(ovTexture);
}

void SCRANTIC::BMPFile::saveFile(const std::string &path) {
    size_t z = 0;
    for (u16 i = 0; i < imageCount; ++i) {
        v8 bmpFile = createRGBABitmapData(uncompressedData, widthList.at(i), heightList.at(i), z);
        z += (widthList.at(i) * heightList.at(i)) / 2;

        std::string num = hexToString(i, std::dec);
        for (size_t j = num.size(); j < 3; ++j) {
            num = "0" + num;
        }
        std::string newFilename = filename + "." + num + ".BMP";

        SCRANTIC::BaseFile::writeFile(bmpFile, newFilename, path);
    }
}


v8 SCRANTIC::BMPFile::repackIntoResource() {

    std::string strings[3] = { "BMP:", "INF:", "BIN:" };

    magic = 0x8000;

    compressionFlag = 2;
    v8 compressedData = LZCCompress(uncompressedData);
    uncompressedSize = uncompressedData.size();
    compressedSize = compressedData.size() + 5;

    imageCount = widthList.size();
    infSize = imageCount * 4 + 2;

    infBinSize = infSize + compressedSize + 16;

    v8 rawData(strings[0].begin(), strings[0].end());
    BaseFile::writeUintLE(rawData, infBinSize);
    BaseFile::writeUintLE(rawData, magic);
    std::copy(strings[1].begin(), strings[1].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, infSize);
    BaseFile::writeUintLE(rawData, imageCount);
    for (u16 i = 0; i < imageCount; ++i) {
        BaseFile::writeUintLE(rawData, widthList.at(i));
    }
    for (u16 i = 0; i < imageCount; ++i) {
        BaseFile::writeUintLE(rawData, heightList.at(i));
    }
    std::copy(strings[2].begin(), strings[2].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, compressedSize);
    BaseFile::writeUintLE(rawData, compressionFlag);
    BaseFile::writeUintLE(rawData, uncompressedSize);
    std::copy(compressedData.begin(), compressedData.end(), std::back_inserter(rawData));

    compressedSize -= 5;

    return rawData;
}



SDL_Texture *SCRANTIC::BMPFile::getImage(SDL_Renderer *renderer, u16 num, SDL_Rect &rect) {
    if ((num >= imageList.size()) || (imageList.at(num) == NULL)) {
        return NULL;
    }

    if (overview == NULL) {
        createOverview();
    }

    if (ovTexture == NULL) {
        ovTexture = SDL_CreateTextureFromSurface(renderer, overview);
        if (ovTexture == NULL) {
            std::cerr << filename << ": Error creating SDL_Texture" << std::endl;
            return NULL;
        }
        SDL_SetTextureBlendMode(ovTexture, SDL_BLENDMODE_BLEND);
    }

    rect = imageRect.at(num);
    return ovTexture;
}

SDL_Rect SCRANTIC::BMPFile::getRect(u16 num) {
    if ((num >= imageList.size()) || (imageList.at(num) == NULL)) {
        return SDL_Rect();
    }

    return imageRect.at(num);
}

void SCRANTIC::BMPFile::createOverview() {
    u16 currentWidth = 0;
    u16 lineHeight = 0;
    u16 maxWidth = 640;
    u16 imgWidth, imgHeight;
    u16 currentY = 0;

    for (size_t i = 0; i < imageList.size(); ++i) {
        imgWidth = imageList.at(i)->w;
        imgHeight = imageList.at(i)->h;
        if (currentWidth + imgWidth < maxWidth) {
            currentWidth += imgWidth;
            if (lineHeight < imgHeight) {
                lineHeight = imgHeight;
            }
        } else {
            currentY += lineHeight;
            lineHeight = imgHeight;
            currentWidth = imgWidth;
        }
    }

    SDL_FreeSurface(overview);

    imageRect.clear();

    overview = SDL_CreateRGBSurface(0, maxWidth, currentY + lineHeight, 8, 0, 0, 0, 0);
    SDL_SetPaletteColors(overview->format->palette, defaultPalette, 0, 256);
    currentWidth = 0;
    currentY = 0;
    lineHeight = 0;

    SDL_Rect rect, target;
    rect.x = 0;
    rect.y = 0;

    for (size_t i = 0; i < imageList.size(); ++i) {
        imgWidth = imageList.at(i)->w;
        imgHeight = imageList.at(i)->h;
        rect.w = imgWidth;
        rect.h = imgHeight;
        target.w = imgWidth;
        target.h = imgHeight;

        if (currentWidth + imgWidth < maxWidth) {
            target.x = currentWidth;
            target.y = currentY;
            SDL_BlitSurface(imageList.at(i), &rect, overview, &target);
            currentWidth += imgWidth;
            if (lineHeight < imgHeight)
                lineHeight = imgHeight;
        } else {
            currentY += lineHeight;
            target.x = 0;
            target.y = currentY;
            SDL_BlitSurface(imageList.at(i), &rect, overview, &target);
            lineHeight = imgHeight;
            currentWidth = imgWidth;
        }
        imageRect.push_back(target);
    }

}

SDL_Texture *SCRANTIC::BMPFile::getOverviewImage(SDL_Renderer *renderer, SDL_Rect &rect) {
    if (overview == NULL) {
        createOverview();
        if (overview == NULL) {
            return NULL;
        }
    }

    if (ovTexture == NULL) {
        ovTexture = SDL_CreateTextureFromSurface(renderer, overview);
        SDL_SetTextureBlendMode(ovTexture, SDL_BLENDMODE_BLEND);
        if (ovTexture == NULL) {
            std::cerr << filename << ": Error creating SDL_Texture" << std::endl;
            return NULL;
        }
    }

    rect.w = overview->w;
    rect.h = overview->h;

    return ovTexture;
}

void SCRANTIC::BMPFile::setPalette(SDL_Color color[], u16 count) {
    if (overview == NULL) {
        return;
    }

    SDL_SetPaletteColors(overview->format->palette, color, 0, 256);
    SDL_DestroyTexture(ovTexture);
    ovTexture = NULL;
}

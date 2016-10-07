#include "BMPFile.h"

SCRANTIC::BMPFile::BMPFile(std::string name, std::vector<u_int8_t> &data)
    : BaseFile(name), overview(NULL), ovTexture(NULL)
{
    std::vector<u_int8_t>::iterator it = data.begin();

    std::string tmp = read_const_string(it, 4);
    if (tmp != "BMP:")
    {
        std::cerr << filename << ": \"BMP:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, infBinSize);
    u_read_le(it, magic);

    tmp = read_const_string(it, 4);
    if (tmp != "INF:")
    {
        std::cerr << filename << ": \"INF:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, infSize);
    u_read_le(it, imageCount);

    u_int16_t word;
    for (int i = 0; i < imageCount; ++i)
    {
        u_read_le(it, word);
        widthList.push_back(word);
    }
    for (int i = 0; i < imageCount; ++i)
    {
        u_read_le(it, word);
        heightList.push_back(word);
    }

    tmp = read_const_string(it, 4);
    if (tmp != "BIN:")
    {
        std::cerr << filename << ": \"BIN:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, binSize);
    binSize -= 5; // substract compressionFlag and uncompressedSize
    u_read_le(it, compressionFlag);
    u_read_le(it, uncompressedSize);

    size_t i = std::distance(data.begin(), it);

    switch (compressionFlag)
    {
    case 0x00: uncompressedData = std::vector<u_int8_t>(it, (it+binSize)); break;
    case 0x01: uncompressedData = RLEDecompress(data, i, uncompressedSize); break;
    case 0x02: uncompressedData = LZWDecompress(data, i, uncompressedSize); break;
    case 0x03: uncompressedData = RLE2Decompress(data, i, uncompressedSize); break;
    default: std::cerr << filename << ": unhandled compression type: " << (int16_t)compressionFlag << std::endl;
    }

    if (uncompressedSize != (u_int32_t)uncompressedData.size())
        std::cerr << filename << ": decompression error: expected size: " << (size_t)uncompressedSize  << " - got " << uncompressedData.size() << " type " << (int16_t)compressionFlag << std::endl;

    if (!uncompressedData.size())
        return;

    //saveFile(uncompressedData, filename, "res/BMP/");

    size_t z = 0;
    bool high = false;
    u_int8_t idx;
    SDL_Surface *image;

    unsigned char *p;

    for (u_int16_t i = 0; i < imageCount; ++i)
    {
        image = SDL_CreateRGBSurface(0, widthList.at(i), heightList.at(i), 8, 0, 0, 0, 0);
        SDL_SetPaletteColors(image->format->palette, defaultPalette, 0, 256);
        p = (unsigned char*)image->pixels;

        for (int y  = 0; y < image->h; ++y)
            for (int x = 0; x < image->w; ++x)
            {
                if (high)
                {
                    high = false;
                    idx = uncompressedData[z] & 0xF;
                    z++;
                }
                else
                {
                    high = true;
                    idx = uncompressedData[z] >> 4;
                }
                p[y * image->w + x] = idx;
            }

        imageList.push_back(image);
    }
    createOverview();
}

SCRANTIC::BMPFile::~BMPFile()
{
    for(auto i = std::begin(imageList); i != std::end(imageList); ++i)
        SDL_FreeSurface(*i);

    SDL_FreeSurface(overview);
    SDL_DestroyTexture(ovTexture);
}

SDL_Texture *SCRANTIC::BMPFile::getImage(SDL_Renderer *renderer, u_int16_t num, SDL_Rect &rect)
{
    if ((num >= imageList.size()) || (imageList.at(num) == NULL))
        return NULL;

    if (overview == NULL)
        createOverview();

    if (ovTexture == NULL)
    {
        ovTexture = SDL_CreateTextureFromSurface(renderer, overview);
        if (ovTexture == NULL)
        {
            std::cerr << filename << ": Error creating SDL_Texture" << std::endl;
            return NULL;
        }
        SDL_SetTextureBlendMode(ovTexture, SDL_BLENDMODE_BLEND);
    }

    rect = imageRect.at(num);
    return ovTexture;
}

SDL_Rect SCRANTIC::BMPFile::getRect(u_int16_t num)
{
    if ((num >= imageList.size()) || (imageList.at(num) == NULL))
        return SDL_Rect();

    return imageRect.at(num);
}

void SCRANTIC::BMPFile::createOverview()
{
    u_int16_t currentWidth = 0;
    u_int16_t lineHeight = 0;
    u_int16_t maxWidth = 640;
    u_int16_t imgWidth, imgHeight;
    u_int16_t currentY = 0;

    for (size_t i = 0; i < imageList.size(); ++i)
    {
        imgWidth = imageList.at(i)->w;
        imgHeight = imageList.at(i)->h;
        if (currentWidth + imgWidth < maxWidth)
        {
            currentWidth += imgWidth;
            if (lineHeight < imgHeight)
                lineHeight = imgHeight;
        }
        else
        {
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

    for (size_t i = 0; i < imageList.size(); ++i)
    {
        imgWidth = imageList.at(i)->w;
        imgHeight = imageList.at(i)->h;
        rect.w = imgWidth;
        rect.h = imgHeight;
        target.w = imgWidth;
        target.h = imgHeight;

        if (currentWidth + imgWidth < maxWidth)
        {
            target.x = currentWidth;
            target.y = currentY;
            SDL_BlitSurface(imageList.at(i), &rect, overview, &target);
            currentWidth += imgWidth;
            if (lineHeight < imgHeight)
                lineHeight = imgHeight;
        }
        else
        {
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

SDL_Texture *SCRANTIC::BMPFile::getOverviewImage(SDL_Renderer *renderer, SDL_Rect &rect)
{
    if (overview == NULL)
    {
        createOverview();
        if (overview == NULL)
            return NULL;
    }

    if (ovTexture == NULL)
    {
        ovTexture = SDL_CreateTextureFromSurface(renderer, overview);
        SDL_SetTextureBlendMode(ovTexture, SDL_BLENDMODE_BLEND);
        if (ovTexture == NULL)
        {
            std::cerr << filename << ": Error creating SDL_Texture" << std::endl;
            return NULL;
        }

        rect.w = overview->w;
        rect.h = overview->h;

        return ovTexture;
    }

    rect.w = overview->w;
    rect.h = overview->h;

    return ovTexture;
}

void SCRANTIC::BMPFile::setPalette(SDL_Color color[], u_int16_t count)
{
    if (overview == NULL)
        return;

    SDL_SetPaletteColors(overview->format->palette, color, 0, 256);
    SDL_DestroyTexture(ovTexture);
    ovTexture = NULL;
}

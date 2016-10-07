#include "SCRFile.h"

SCRANTIC::SCRFile::SCRFile(std::string name, std::vector<u_int8_t> &data)
    : BaseFile(name), image(NULL), texture(NULL)
{
    std::vector<u_int8_t>::iterator it = data.begin();

    std::string tmp = read_const_string(it, 4);
    if (tmp != "SCR:")
    {
        std::cerr << filename << ": \"SCR:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, dimBinSize);
    u_read_le(it, magic);

    tmp = read_const_string(it, 4);
    if (tmp != "DIM:")
    {
        std::cerr << filename << ": \"DIM:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, dimSize);
    u_read_le(it, width);
    u_read_le(it, height);

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
        std::cerr << filename << ": decompression error: expected size: " << (size_t)uncompressedSize  << " - got " << uncompressedData.size() << std::endl;

    if (!uncompressedData.size())
        return;

    //saveFile(uncompressedData, filename, "res/SCR/");

    image = SDL_CreateRGBSurface(0, width, height, 8, 0, 0, 0, 0);
    SDL_SetPaletteColors(image->format->palette, defaultPalette, 0, 256);

    size_t z = 0;
    bool high = false;
    u_int8_t idx;
    unsigned char *p = (unsigned char*)image->pixels;

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
}

SCRANTIC::SCRFile::~SCRFile()
{
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(image);
}

void SCRANTIC::SCRFile::setPalette(SDL_Color color[], u_int16_t count)
{
    if (image == NULL)
        return;

    SDL_SetPaletteColors(image->format->palette, color, 0, 256);
    SDL_DestroyTexture(texture);
    texture = NULL;
}

SDL_Texture *SCRANTIC::SCRFile::getImage(SDL_Renderer *renderer, SDL_Rect &rect)
{
    if (image == NULL)
        return NULL;

    if (texture == NULL)
    {
        texture = SDL_CreateTextureFromSurface(renderer, image);
        if (texture == NULL)
        {
            std::cerr << filename << ": Error creating SDL_Texture" << std::endl;
            return NULL;
        }

        rect.w = image->w;
        rect.h = image->h;

        return texture;
    }

    rect.w = image->w;
    rect.h = image->h;

    return texture;
}

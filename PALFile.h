#ifndef PALFILE_H
#define PALFILE_H

#include "BaseFile.h"
#include <tuple>

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace SCRANTIC {

class PALFile : public BaseFile
{
protected:
    u16 vgaSize;
    u16 magic;  // 0x8000
    u32 palCount;
    std::vector<SDL_Color> palette;

public:
    PALFile(const std::string &name, v8 &data);
    explicit PALFile(const std::string &filename);
    v8 repackIntoResource() override;
    void saveFile(const std::string &path);
    SDL_Color *getPalette() { return &palette[0]; }
};

}

#endif // PALFILE_H

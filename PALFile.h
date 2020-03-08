#ifndef PALFILE_H
#define PALFILE_H

#include "BaseFile.h"
#include <tuple>

namespace SCRANTIC {

class PALFile : public BaseFile
{
protected:
    u16 vgaSize;
    u16 magic;  // 0x8000
    u32 palCount;
    //std::vector< std::tuple< u8, u8, u8 > > palette;
    std::vector<SDL_Color> palette;
public:
    PALFile(const std::string &name, v8 &data);
    //std::vector< std::tuple< u8, u8, u8 > > getPalette() { return palette; }
    SDL_Color *getPalette() { return &palette[0]; }
};

}

#endif // PALFILE_H

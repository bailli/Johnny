#ifndef PALFILE_H
#define PALFILE_H

#include "BaseFile.h"
#include <tuple>

namespace SCRANTIC {

class PALFile : public BaseFile
{
protected:
    u_int16_t vgaSize;
    u_int16_t magic;  // 0x8000
    u_int32_t palCount;
    //std::vector< std::tuple< u_int8_t, u_int8_t, u_int8_t > > palette;
    std::vector<SDL_Color> palette;
public:
    PALFile(std::string name, std::vector<u_int8_t> &data);
    //std::vector< std::tuple< u_int8_t, u_int8_t, u_int8_t > > getPalette() { return palette; }
    SDL_Color *getPalette() { return &palette[0]; }
};

}

#endif // PALFILE_H

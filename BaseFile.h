#ifndef BASEFILE_H
#define BASEFILE_H

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace SCRANTIC {

// TTM instructions
#define CMD_DRAW_BACKGROUND  0x0080 // only after "Select Image Slot" - clear slot?!
#define CMD_PURGE            0x0110 // still not sure what really gets "purged"
#define CMD_UPDATE           0x0FF0
#define CMD_DELAY            0x1020 // how long is delay 0 ?
#define CMD_SEL_SLOT_IMG     0x1050
#define CMD_SEL_SLOT_PAL     0x1060
#define CMD_UNK_1100         0x1100 // called 5 times always before 0xA600
#define CMD_SET_SCENE        0x1110
#define CMD_UNK_1120         0x1120 // always(?) before "Save New Image" parm 0/1 (once 2 - mistake?) does this actually clear the saved image?
#define CMD_JMP_SCENE        0x1200 // param very often own scene number - not always
#define CMD_SET_COLOR        0x2000 // Set Bg/Fg color? (once 0xcf 0xcf - mistake?)
#define CMD_SET_FRAME_1      0x2010 // param always 0x0 0x0 usually before Select Image Slot/Load Bitmap
#define CMD_UNK_2020         0x2020 // called often in "xyz timer"
#define CMD_CLIP_REGION      0x4000 // clip region for sprites ATTENTION x1, y1, x2, y2 - not width/height!
#define CMD_SAVE_IMAGE       0x4200 // save to last image?
#define CMD_SAVE_IMAGE_NEW   0x4210 // save to new image
#define CMD_DRAW_PIXEL       0xA000 // draw pixel at x,y
#define CMD_UNK_A050         0xA050 // called only once in LILIPUTS ROW ASHORE (2: Unkown 0xA050 00c2 007b 00af 008e)
#define CMD_UNK_A060         0xA060 // called only once (5: Unkown 0xA060 00c2 007b 00af 008e)
#define CMD_DRAW_LINE        0xA0A0
#define CMD_DRAW_RECTANGLE   0xA100 // draw rectangle?! (colors from set frame?!)
#define CMD_DRAW_ELLIPSE     0xA400
#define CMD_DRAW_SPRITE      0xA500
#define CMD_DRAW_SPRITE_MIRROR 0xA520 // mirrored
#define CMD_CLEAR_RENDERER   0xA600 // param 0/1/2 (count: 6126/673/15) empty sprite list?!
#define CMD_DRAW_SCREEN      0xB600 // called 6 times params: rect + 0x2 + 0x1 ?!
#define CMD_PLAY_SOUND       0xC050 // not all sounds used?!
#define CMD_LOAD_SCREEN      0xF010
#define CMD_LOAD_BITMAP      0xF020
#define CMD_LOAD_PALETTE     0xF050

// ADS instructions
#define CMD_UNK_1070         0x1070 // called only once before 0x1520 (0x1520 might be third param?)
                                    // 36: Unkown 0x1070 0004 0005
                                    // 37: Unkown 0x1520
#define CMD_ADD_INIT_TTM     0x1330 // Init for TTM Res $1 Scene $2 - why is scene needed?
#define CMD_COND_MOVIE       0x1350 // more like "do while ttm/scene last played"
                                    // play the following only, but always, after res/scene
#define CMD_SKIP_IF_LAST     0x1360 // this seems like an actual skip if res/scene was lastplayed
#define CMD_UNK_1370         0x1370 // 2 Params: TTM and Scene ?
#define CMD_OR_SKIP          0x1420 // always after/between SKIPNEXT2  OR condition FOR CMD_SKIP_NEXT_IF_2 ?
#define CMD_OR               0x1430 // already attached to CMD_COND_MOVIE
#define CMD_PLAY_MOVIE       0x1510 // play one movie from rand list OR play movie list
#define CMD_UNK_1520         0x1520 // only called once
                                    // no params; Add TTM follows 2005 0004 0016 0000 0001
#define CMD_ADD_TTM          0x2005 // $1: res $2: scene $3: ??? $4: repeat --- does no longer force init scene 0
#define CMD_KILL_TTM         0x2010 // kill TTM
#define CMD_RANDOM_START     0x3010 // add following movies to random list
#define CMD_RANDOM_UNKNOWN_1 0x3020 // params 5 (with "Set Frame")/2 (once with 1 TTM)/1 (4x with 3 TTM)
#define CMD_RANDOM_END       0x30FF // rand list end
// CMD_SET_WINDOW_0     0x4000      // called 4 times - 2x at "end"
// CMD_LOAD_SCREEN      0xF010      // called 67 times
#define CMD_PLAY_ADS_MOVIE   0xF200 //  0: Select Scene 0001 MUN. AMB. POS.A  SW
                                    //  1: Unkown 0xF200 000e <-- play movie no. 0x000e ?
#define CMD_UNK_FFFF         0xFFFF // Part of Command "Load Screen"?

//return values for getNextCommand
#define CMD_INTER_NOTFOUND   0x0001
#define CMD_INTER_END        0x0002


struct Command
{
    u_int16_t opcode;
    std::vector<u_int16_t> data;
    std::string name;
};

class BaseFile
{
protected:
    static std::vector<u_int8_t> RLEDecompress(std::vector<u_int8_t> const &compressedData, size_t offset = 0, u_int32_t size = 0);
    static std::vector<u_int8_t> RLE2Decompress(std::vector<u_int8_t> const &compressedData, size_t offset = 0, u_int32_t size = 0);
    static std::vector<u_int8_t> LZWDecompress(std::vector<u_int8_t> const &compressedData, size_t offset = 0, u_int32_t size = 0);
    SDL_Color defaultPalette[256];

public:
    BaseFile(std::string name);
    ~BaseFile();
    std::string filename;
    static std::string commandToString(Command cmd);

    static std::string read_string(std::ifstream *in, u_int8_t length = 0);
    static std::string read_string(std::vector<u_int8_t>::iterator &it, u_int8_t length = 0);
    static std::string read_const_string(std::ifstream *in, u_int8_t length);
    static std::string read_const_string(std::vector<u_int8_t>::iterator &it, u_int8_t length);
    static u_int16_t readBits(std::vector<u_int8_t> const &data, size_t &bytePos, u_int8_t &bitPos, u_int16_t bits);
    template < typename T > static void u_read_le(std::ifstream *in, T &var);
    template < typename T > static void u_read_le(std::vector<u_int8_t>::iterator &it, T &var);
    template < typename T > static std::string hex_to_string(T t, std::ios_base & (*f)(std::ios_base&));
    static void saveFile(const std::vector<u_int8_t> &data, std::string name, std::string path = "");
};


}

template < typename T >
void SCRANTIC::BaseFile::u_read_le(std::ifstream *in, T &var)
{
    if (!in->is_open())
        return;

    u_int8_t size = sizeof(var);
    u_int8_t byte;
    var = 0;

    for (u_int8_t i = 0; i < size; ++i)
    {
        in->read((char*)&byte, 1);
        var |= (byte << (i * 8));
    }
}

template < typename T >
void SCRANTIC::BaseFile::u_read_le(std::vector<u_int8_t>::iterator &it, T &var)
{
    u_int8_t size = sizeof(var);
    var = 0;

    for (u_int8_t i = 0; i < size; ++i)
    {
        var |= (*it << (i * 8));
        ++it;
    }
}

template <class T>
std::string SCRANTIC::BaseFile::hex_to_string(T t, std::ios_base & (*f)(std::ios_base&))
{
  std::ostringstream oss;
  oss << f << t;
  return oss.str();
}


#endif // BASEFILE_H

/*#define CMD_SAVE_BACKGROUND  0x0020 // not called
#define CMD_FADE_OUT         0x4110 // not called
#define CMD_FADE_IN          0x4120 // not called
#define CMD_DRAW_SPRITE_1    0xA510 // not called
#define CMD_DRAW_SPRITE_3    0xA530 // not called
#define CMD_LOAD_SOUND       0xC020 // not called
#define CMD_SELECT_SOUND     0xC030 // not called
#define CMD_DESELECT_SOUND   0xC040 // not called
#define CMD_STOP_SOUND       0xC060 // not called
#define CMD_PLAY_MOVIE_2     0x1515 // not called*/

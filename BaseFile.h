#ifndef BASEFILE_H
#define BASEFILE_H

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include "types.h"

namespace SCRANTIC {

// TTM instructions
#define CMD_CLEAR_IMGSLOT    0x0080 // only after "Select Image Slot" - clear slot?! -- old define CMD_DRAW_BACKGROUND / no params => seems right
#define CMD_PURGE            0x0110 // still not sure what really gets "purged" / no params
#define CMD_UPDATE           0x0FF0 // no params

#define CMD_DELAY            0x1020 // how long is delay 0 ? / 1 param: delay
#define CMD_SEL_SLOT_IMG     0x1050 // 1 param: slot number
#define CMD_SEL_SLOT_PAL     0x1060 // 1 param: slot number
#define CMD_SET_SCENE_LABEL  0x1100 // called 5 times always before 0xA600 also SET_SCENE_LABEL / 1 param: scene number
#define CMD_SET_SCENE        0x1110 // 1 param: scene number
#define CMD_UNK_1120         0x1120 // always(?) before "Save New Image" parm 0/1 (once 2 - mistake?) does this actually clear the saved image? / 1 param
#define CMD_JMP_SCENE        0x1200 // param very often own scene number - not always / 1 param scene number
#define CMD_CLEAR_RENDERER   0xA600 // param 0/1/2 (count: 6126/673/15) empty sprite list?! / 1 param
#define CMD_PLAY_SOUND       0xC050 // not all sounds used?! / 1 param
#define CMD_LOAD_SCREEN      0xF010 // 1 string param
#define CMD_LOAD_BITMAP      0xF020 // 1 string param
#define CMD_LOAD_PALETTE     0xF050 // 1 string param

#define CMD_SET_COLOR        0x2000 // Set Bg/Fg color? (once 0xcf 0xcf - mistake?) / 2 params fore/background colour
#define CMD_UNK_2010         0x2010 // param always 0x0 0x0 usually before Select Image Slot/Load Bitmap -- old define CMD_SET_FRAME_1 / 2 params
#define CMD_TIMER            0x2020 // called often in "xyz timer" / 2 params wait count & delay
#define CMD_DRAW_PIXEL       0xA000 // draw pixel at x,y / 2 params

#define CMD_CLIP_REGION      0x4000 // clip region for sprites ATTENTION x1, y1, x2, y2 - not width/height! / 4 params
#define CMD_SAVE_IMAGE       0x4200 // save to last image? / 4 params x,y,w,h
#define CMD_SAVE_IMAGE_NEW   0x4210 // save to new image / 4 params x,y,w,h
#define CMD_UNK_A050         0xA050 // called only once in LILIPUTS ROW ASHORE (2: Unkown 0xA050 00c2 007b 00af 008e) freeze part of screen? / 4 params => removing this command the ship is "kept" after sailing away
#define CMD_UNK_A060         0xA060 // called only once (5: Unkown 0xA060 00c2 007b 00af 008e) unfreeze screen --- probably wrong / 4 params => removing this command seems to do nothing?!
#define CMD_DRAW_LINE        0xA0A0 // 4 params
#define CMD_DRAW_RECTANGLE   0xA100 // draw rectangle?! (colors from set frame?!) / 4 params
#define CMD_DRAW_ELLIPSE     0xA400 // 4 params
#define CMD_DRAW_SPRITE      0xA500 // 4 params
//#define CMD_DRAW_SPRITE_VMIRROR 0xA510 // 4 params
#define CMD_DRAW_SPRITE_MIRROR 0xA520 // mirrored / 4 params
//#define CMD_DRAW_SPRITE_HVMIRROR 0xA510 // 4 params ?

#define CMD_UNK_B600         0xB600 // called 6 times params: rect + 0x2 +  0x1 ?! -- old define CMD_DRAW_SCREEN / 6 params

// ADS instructions
#define CMD_UNK_1070         0x1070 // called only once before 0x1520 (0x1520 might be third param?)
                                    // don't follow to label for ttm/scene ?
                                    // 36: Unkown 0x1070 0004 0005
                                    // 37: Unkown 0x1520
#define CMD_ADD_INIT_TTM     0x1330 // Init for TTM Res $1 Scene $2 - why is scene needed?
#define CMD_AFTER_SCENE      0x1350
#define CMD_TTM_LABEL        0x1350 // more like "do while ttm/scene last played"
                                    // play the following only, but always, after res/scene
#define CMD_SKIP_IF_PLAYED   0x1360
#define CMD_SKIP_IF_LAST     0x1360 // this seems like an actual skip if res/scene was lastplayed
#define CMD_ONLY_IF_PLAYED   0x1370
#define CMD_UNK_1370         0x1370 // 2 Params: TTM and Scene ? ==> Next command only with or after TTM/scene
#define CMD_OR_SKIP          0x1420 // always after/between SKIPNEXT2  OR condition FOR CMD_SKIP_NEXT_IF_2 ?
#define CMD_OR               0x1430 // already attached to CMD_COND_MOVIE
#define CMD_OR_AFTER         0x1430 // already attached to CMD_COND_MOVIE
#define CMD_PLAY_MOVIE       0x1510 // play one movie from rand list OR play movie list
#define CMD_UNK_1520         0x1520 // only called once
                                    // no params; Add TTM follows 2005 0004 0016 0000 0001
#define CMD_ADD_TTM          0x2005 // $1: res $2: scene $3: repeat $4: ??? --- does no longer force init scene 0
#define CMD_KILL_TTM         0x2010 // kill TTM
#define CMD_RANDOM_START     0x3010 // add following movies to random list
#define CMD_ZERO_CHANCE      0x3020
#define CMD_UNK_3020         0x3020 // params 5 (with "Set Frame")/2 (once with 1 TTM)/1 (4x with 3 TTM) -- old define CMD_RANDOM_UNKNOWN_1  ==> random chance that no ttm will be selected?
#define CMD_RANDOM_END       0x30FF // rand list end
#define CMD_UNK_LABEL        0x4000
#define CMD_UNK_4000         0x4000 // called 4 times - 2x at "end"
#define CMD_UNK_F010         0xF010 // called 67 times ==> 0xF010 0xFFFF end current script
#define CMD_PLAY_ADS_MOVIE   0xF200 //  0: Select Scene 0001 MUN. AMB. POS.A  SW
                                    //  1: Unkown 0xF200 000e <-- play movie no. 0x000e ?
#define CMD_UNK_FFFF         0xFFFF // Part of Command "0xF010?


struct BaseFileException : public std::exception {
   const char * what () const throw () {
      return "Magic string not found!";
   }
};

struct Command {
    u16 opcode;
    v16 data;
    std::string name;
};

class BaseFile {
protected:
    void assertString(v8::iterator &it, std::string expectedString);

public:
    explicit BaseFile(const std::string &name);
    virtual ~BaseFile();

    std::string filename;
    virtual void saveFile(const std::string &path) {};
    virtual v8 repackIntoResource() { return v8{}; };

    static std::string commandToString(Command cmd, bool ads = false);

    static std::string readString(std::ifstream *in, u8 length = 0, char delimiter = '\0');
    static std::string readString(v8::iterator &it, u8 length = 0, char delimiter = '\0');
    static std::string readConstString(std::ifstream *in, u8 length);
    static std::string readConstString(v8::iterator &it, u8 length);

    static void writeFile(const v8 &data, std::string &name, std::string path = "");
    static void writeFile(const std::string &data, std::string &name, std::string path = "");

    static v8 readFile(const std::string &filename);

    template < typename T > static void readUintLE(std::ifstream *in, T &var);
    template < typename T > static void readUintLE(v8::iterator &it, T &var);
    template < typename T > static std::string hexToString(T t, std::ios_base & (*f)(std::ios_base&),int pad = 0);

    template < typename T > static void writeUintLE(v8 &data, T &var);
    template < typename T > static void writeUintLE(std::ofstream *out, T &var);
};

}

template < typename T >
void SCRANTIC::BaseFile::readUintLE(std::ifstream *in, T &var) {
    if (!in->is_open()) {
        return;
    }

    size_t size = sizeof(var);
    u8 byte;
    var = 0;

    for (u8 i = 0; i < size; ++i) {
        in->read((char*)&byte, 1);
        var |= (byte << (i * 8));
    }
}

template < typename T >
void SCRANTIC::BaseFile::readUintLE(v8::iterator &it, T &var) {
    size_t size = sizeof(var);
    var = 0;

    for (u8 i = 0; i < size; ++i) {
        var |= (*it << (i * 8));
        ++it;
    }
}

template <class T>
std::string SCRANTIC::BaseFile::hexToString(T t, std::ios_base & (*f)(std::ios_base&), int pad) {
    std::ostringstream oss;
    oss << f << t;
    if (pad == 0) {
        return oss.str();
    }
    std::string result = oss.str();
    for (int i = result.size(); i < pad; ++i) {
        result = "0" + result;
    }
    return result;
}

template < typename T >
void SCRANTIC::BaseFile::writeUintLE(v8 &data, T &var) {
    size_t size = sizeof(var);
    for (u8 i = 0; i < size; ++i) {
        data.push_back((var >> (8*i)) & 0xFF);
    }
}

template < typename T >
void SCRANTIC::BaseFile::writeUintLE(std::ofstream *out, T &var) {
    if (!out->is_open()) {
        return;
    }

    size_t size = sizeof(var);
    u8 byte;
    var = 0;

    for (u8 i = 0; i < size; ++i) {
        out->write((char*)&byte, 1);
        var |= (byte << (i * 8));
    }
}

#endif // BASEFILE_H


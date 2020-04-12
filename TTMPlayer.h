#ifndef TTMPLAYER_H
#define TTMPLAYER_H

#include "TTMFile.h"
#include "BMPFile.h"
#include "RESFile.h"
#include "ADSFile.h"

#include "defines.h"

#include <list>

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace SCRANTIC {

struct SceneItem {
    SDL_Texture *tex;
    SDL_Rect src;
    SDL_Rect dest;
    u16 num;
    u8 flags;
    std::pair<u16, u16> color;
    i8 itemType;
};

class TTMPlayer {

protected:
    std::vector<Command> script;
    std::vector<Command>::iterator scriptPos;

    std::list<SceneItem> items;
    std::list<SceneItem> queuedItems;
    std::list<SceneItem>::iterator itemPos;

    SDL_Color *palette;
    std::pair<u16, u16> currentColor;

    std::string name;

    u16 resNo;
    u16 sceneNo;
    u16 originalScene;
    u16 delay;
    u16 remainingDelay;
    u16 waitCount;
    u16 waitDelay;
    u16 imgSlot;
    i16 audioSample;
    i16 repeat;
    i16 maxTicks;

    i32 jumpToScript;

    SDL_Renderer *renderer;

    bool clipRegion;
    bool alreadySaved;
    bool saveNewImage;
    bool saveImage;
    bool isDone;
    bool toBeKilled;
    bool selfDestruct;
    bool selfDestructActive;

    SDL_Rect clipRect;
    SDL_Rect saveRect;

    BMPFile **images;
    RESFile *res;
    TTMFile *ttm;

    std::string screen;

public:
    u16 getDelay() { return delay; };
    u16 getRemainigDelay(u32 ticks);
    SDL_Rect getClipRect() { return clipRect; }
    i16 getSample() { i16 tmp = audioSample; audioSample = -1; return tmp; }
    std::string getSCRName() { return screen; }
    bool isFinished() { return isDone; }
    bool isClipped() { return clipRegion; }
    u8 needsSave();

    void kill() { toBeKilled = true; }
    u16 getHash() { return SCRANTIC::ADSFile::makeHash(resNo, originalScene); }

    //Needs to be freed
    SDL_Texture *savedImage;
    SDL_Texture *fg;

    void advanceScript();
    void renderForeground();

    TTMPlayer(const std::string &ttmName, u16 resNo, u16 scene, i16 repeatCount, RESFile *resFile, BMPFile **images, SDL_Color *pal, SDL_Renderer *rendererContext);
    ~TTMPlayer();
};

}

#endif // TTMPLAYER_H

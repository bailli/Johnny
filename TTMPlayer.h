#ifndef TTMPLAYER_H
#define TTMPLAYER_H

#include "TTMFile.h"
#include "BMPFile.h"
#include "RESFile.h"

#include "defines.h"

#include <list>

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace SCRANTIC {

struct SceneItem
{
    SDL_Texture *tex;
    SDL_Rect src;
    SDL_Rect dest;
    u_int16_t num;
    u_int8_t flags;
    std::pair<u_int16_t, u_int16_t> color;
    int8_t itemType;
};

class TTMPlayer
{
protected:
    std::vector<Command> script;
    std::vector<Command>::iterator scriptPos;

    std::list<SceneItem> items;
    std::list<SceneItem> queuedItems;
    std::list<SceneItem>::iterator itemPos;

    SDL_Color *palette;
    std::pair<u_int16_t, u_int16_t> currentColor;

    std::string name;

    u_int16_t resNo;
    u_int16_t sceneNo;
    u_int16_t originalScene;
    u_int16_t delay;
    u_int16_t remainingDelay;
    u_int16_t imgSlot;
    int16_t audioSample;
    int16_t repeat;

    int32_t jumpToScript;

    SDL_Renderer *renderer;

    bool clipRegion;
    bool alreadySaved;
    bool saveNewImage;
    bool saveImage;
    bool isDone;
    bool toBeKilled;

    SDL_Rect clipRect;
    SDL_Rect saveRect;

    BMPFile **images;
    RESFile *res;
    TTMFile *ttm;

    std::string screen;

public:
    u_int16_t getDelay();
    u_int16_t getRemainigDelay(u_int32_t ticks);
    SDL_Rect getClipRect() { return clipRect; }
    int16_t getSample() { int16_t tmp = audioSample; audioSample = -1; return tmp; }
    std::string getSCRName() { return screen; }
    bool isFinished() { return isDone; }
    bool isClipped() { return clipRegion; }
    u_int8_t needsSave();

    void kill() { toBeKilled = true; }
    std::pair<u_int16_t, u_int16_t> getHash() { return std::make_pair(resNo, originalScene); }

    //Needs to be freed
    SDL_Texture *savedImage;
    SDL_Texture *fg;

    void advanceScript();
    void renderForeground();

    TTMPlayer(std::string ttmName, u_int16_t resNo, u_int16_t scene, int16_t repeatCount, RESFile *resFile, BMPFile **images, SDL_Color *pal, SDL_Renderer *rendererContext);
    ~TTMPlayer();
};

}

#endif // TTMPLAYER_H

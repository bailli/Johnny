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

enum ttmPlayerStatus { ttmError = 0, ttmUpdate = 1, ttmFinished = 2, ttmSaveImage = 4,
                       ttmSaveNew = 8, ttmClipRegion = 16, ttmPlaySound = 32, ttmLoadScreen = 64, ttmPurge = 128 };

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

    std::pair<u_int16_t, u_int16_t> currentColor;
    std::list<SceneItem> items;
    std::list<SceneItem> queuedItems;
    std::list<SceneItem>::iterator itemPos;

    SDL_Color *palette;

    std::string name;

    u_int16_t resNo;
    u_int16_t sceneNo;

    u_int16_t delay;
    u_int16_t imgSlot;
    //u_int16_t palSlot;

    u_int8_t audioSample;

    SDL_Renderer *renderer;

    bool clipRegion;
    bool alreadySaved;

    SDL_Rect clipRect;
    SDL_Rect saveRect;

    //needs to be shared among TTMPlayers?
    BMPFile **images;

    RESFile *res;

    std::string screen;

    u_int16_t lastResult;

public:
    u_int16_t getDelay();
    SDL_Rect getClipRect() { return clipRect; }
    SDL_Rect getSaveRect() { return saveRect; }
    u_int8_t getSample() { return audioSample; }
    std::string getSCRName() { return screen; }
    SceneItem getSceneItem(bool reset = false);
    u_int16_t getLastResult() { return lastResult; }
    void copiedImage() { lastResult = (lastResult |= ttmSaveImage) ^ ttmSaveImage; }
    std::pair<u_int16_t, u_int16_t> getHash() { return std::make_pair(resNo, sceneNo); }

    //Needs to be freed
    SDL_Texture *savedImage;
    SDL_Texture *fg;

    u_int16_t advanceScript();
    void renderForeground();

    TTMPlayer(std::string ttmName, u_int16_t resNo, u_int16_t scene, RESFile *resFile, BMPFile **images, SDL_Color *pal, SDL_Renderer *rendererContext);
    ~TTMPlayer();
};

}

#endif // TTMPLAYER_H

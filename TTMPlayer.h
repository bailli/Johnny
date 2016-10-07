#ifndef TTMPLAYER_H
#define TTMPLAYER_H

#include "TTMFile.h"
#include "SCRFile.h"
#include "BMPFile.h"
#include "RESFile.h"

#include <list>

#ifdef WIN32
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

namespace SCRANTIC {

#define RENDERITEM_SPRITE 0
#define RENDERITEM_LINE 1
#define RENDERITEM_RECT 2
#define RENDERITEM_ELLIPSE 3

#define RENDERFLAG_MIRROR 0x1

#define MAX_IMAGES 10
#define MAX_AUDIO 24

enum ttmPlayerStatus { ttmError = 0, ttmUpdate = 1, ttmFinished = 2, ttmSaveImage = 4,
                       ttmSaveNew = 8, ttmClipRegion = 16, ttmPlaySound = 32, ttmLoadScreen = 64 };

struct SceneItem
{
    SDL_Texture *tex;
    SDL_Rect src;
    SDL_Rect dest;
    u_int16_t num;
    u_int8_t flags;
    std::pair<u_int16_t, u_int16_t> color;
    u_int8_t itemType;
};

class TTMPlayer
{
protected:
    std::vector<Command> script;
    std::vector<Command>::iterator scriptPos;

    std::pair<u_int16_t, u_int16_t> currentColor;
    std::list<SceneItem> items;
    std::list<SceneItem> queuedItems;

    std::string name;

    u_int16_t delay;
    u_int16_t imgSlot;
    //u_int16_t palSlot;

    u_int8_t audioSample;

    SDL_Renderer *renderer;

    bool clipRegion;

    SDL_Rect clipRect;
    SDL_Rect saveRect;

    BMPFile *images[MAX_IMAGES];

    RESFile *res;

    std::string screen;

public:
    u_int16_t getDelay();
    SDL_Rect getClipRegion() { return clipRect; }
    std::string getScreen() { return screen; }

    u_int16_t advanceScript();

    TTMPlayer(TTMFile *ttm, u_int16_t scene, RESFile *resFile, SDL_Renderer *rendererContext);
    ~TTMPlayer();
};

}

#endif // TTMPLAYER_H

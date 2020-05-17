#ifndef ROBINSONCOMPOSITOR_H
#define ROBINSONCOMPOSITOR_H

#include "types.h"

#include "SCRFile.h"
#include "BMPFile.h"
#include "TTMPlayer.h"

#ifdef WIN32
#include <SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif

namespace SCRANTIC {

class RobinsonCompositor
{
private:
    SDL_Renderer *renderer;

    int width;
    int height;

    bool isLargeIsland;
    bool isNight;

    RESFile *resources;

    BMPFile *backgroundBMP;
    BMPFile *holidayBMP;
    BMPFile *raftBMP;
    BMPFile *cloudsBMP;

    SCRFile *splashSCR;
    std::vector<SCRFile *> oceanSCRList;
    SCRFile *oceanNightSCR;
    SCRFile *oceanSCR;
    SCRFile *screenSCR;

    i8 animationCycle;
    i8 islandPos;
    SDL_Point islandTrunk;
    SDL_Point absoluteIslandTrunkPos;

    SDL_Texture *oceanTexture;
    SDL_Texture *bgTexture;
    SDL_Texture *fgTexture;
    SDL_Texture *saveTexture;
    SDL_Texture *rendererTarget;

    SDL_Rect fullScreenRect;

    RobinsonCompositor(const RobinsonCompositor& other);

    void renderSpriteNumAtPos(BMPFile *bmp, u16 num, i32 x, i32 y);
    void renderFullScreenSCR(SCRFile *scr);
    void animateBackground();

public:
    RobinsonCompositor(SDL_Renderer* renderer, int width, int height, RESFile *resources);
    ~RobinsonCompositor();

    void displaySplash();
    void reset();
    void render(std::list<TTMPlayer *>::iterator begin, std::list<TTMPlayer *>::iterator end);
    void setScreen(const std::string &screen);

    void setNight(bool night) { isNight = night; }
    void setOcean(int num) { oceanSCR = oceanSCRList[num]; }
    void setLargeIsland(bool large) { isLargeIsland = large; }
    void setAbsoluteIslandPos(SDL_Point *pos) { absoluteIslandTrunkPos = *pos; }

    bool getNight() { return isNight; }
    bool getLargeIsland() { return isLargeIsland; }
};

}

#endif // ROBINSONCOMPOSITOR_H

#ifndef ROBINSON_H
#define ROBINSON_H

#include "RESFile.h"
#include "ADSFile.h"
#include "SCRFile.h"
#include "BMPFile.h"
#include "PALFile.h"
#include "TTMPlayer.h"
#include "RIFFPlayer.h"
#include "RobinsonMenu.h"

#ifdef WIN32
#include <SDL_ttf.h>
#else
#include <SDL2/SDL_ttf.h>
#endif

namespace SCRANTIC {

class Robinson
{
private:
    Robinson(Robinson &C);

    RESFile *res;
    ADSFile *ads;

    RIFFPlayer *audioPlayer;
    RobinsonMenu *menu;

    SDL_Renderer *renderer;
    SDL_Texture *rendererTarget;

    SDL_Color *palette;

    i8 animationCycle;
    i8 islandPos;
    SDL_Point islandTrunk;

    bool islandNight;
    bool islandLarge;
    bool movieRunning;
    bool movieLastRun;
    bool renderMenu;

    // need to be freed
    SDL_Texture *bgTexture;
    SDL_Texture *fgTexture;
    SDL_Texture *saveTexture;

    //freed by SCRFile
    SDL_Texture *oceanTexture;
    SDL_Texture *oceanNightTexture;
    SDL_Texture *scrTexture;

    //freed by BMPFile
    BMPFile *backgroundBMP;
    BMPFile *holidayBMP;
    BMPFile *raftBMP;

    std::vector<u16> lastHashes;

    // lots of rects...
    SDL_Rect fullRect;
    SDL_Rect oceanRect;
    SDL_Rect screenRect;

    //u16 currentMovie;
    std::string name;

    std::list<TTMPlayer *> ttmScenes;

    BMPFile *images[MAX_IMAGES];

    u16 delay;
    u16 currentMovie;
    u16 queuedMovie;
    size_t queuedPos;

    u32 delayTicks;

    void resetPlayer();

    void renderBackgroundAtPos(u16 num, i32 x, i32 y, bool raft = false, bool holiday = false);
    void animateBackground();
    void displaySplash();

    void addTTM(Command cmd);
    void runTTMs();
    void runADSBlock(bool togetherWith, u16 movie, u16 hash, u16 num = 0);

public:
    Robinson(const std::string &path, bool readUnpacked);
    ~Robinson();

    bool navigateMenu(SDL_Keycode key);

    void initRenderer(SDL_Renderer *rendererSDL, TTF_Font *font);

    void advanceScripts();
    void render();

    bool isMenuOpen() { return renderMenu; }
    bool isMovieRunning() { return movieRunning; }

    bool loadMovie(const std::string &adsName, u16 num);
    void startMovie();

    u32 getCurrentDelay() { return delay; }
    void displayMenu(bool show) { renderMenu = show; }
};

}

#endif // ROBINSON_H

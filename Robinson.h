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
#include "RobinsonCompositor.h"

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
    RobinsonCompositor *compositor;

    SDL_Renderer *renderer;

    SDL_Color *palette;

    bool movieRunning;
    bool movieFirstRun;
    bool movieLastRun;
    bool renderMenu;

    //u16 currentMovie;
    std::string adsFileName;

    std::list<TTMPlayer *> ttmScenes;
    std::list<std::pair<u16, bool>> movieQueue;
    std::list<u16> lastHashes;
    std::list<u16> killedHashes;

    BMPFile *images[MAX_IMAGES];

    u16 delay;
    u16 currentMovie;

    u32 delayTicks;

    void resetPlayer();

    void addTTM(Command cmd);
    void runTTMs();
    void runADSBlock(bool togetherWith, u16 movie, u16 hash, u16 num = 0, bool skipToPlayAds = false);

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
    void displaySplash();
};

}

#endif // ROBINSON_H

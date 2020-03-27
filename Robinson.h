#ifndef ROBINSON_H
#define ROBINSON_H

#include "RESFile.h"
#include "ADSFile.h"
#include "SCRFile.h"
#include "BMPFile.h"
#include "PALFile.h"
#include "TTMPlayer.h"
#include "RIFFPlayer.h"

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

protected:
    //menu stuff
    std::map<std::string, SDL_Texture *> menuScreen;
    std::map<std::string, std::map<u16, SDL_Rect> > menuPos;
    std::string currentMenuScreen;
    size_t currentMenuPos;
    bool renderMenu;

    RESFile *res;
    ADSFile *ads;
    std::vector<Command> script;
    size_t scriptPos;
    std::multimap<std::pair<u16, u16>, size_t> labels;

    RIFFPlayer *audioPlayer;

    SDL_Renderer *renderer;
    SDL_Texture *rendererTarget;

    SDL_Color *palette;

    i8 animationCycle;
    i8 islandPos;
    SDL_Point islandTrunk;

    bool islandNight;
    bool islandLarge;
    bool movieRunning;

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

    std::list<std::pair<u16, u16> > lastTTMs;

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

    void menuRenderer();
    void renderBackgroundAtPos(u16 num, i32 x, i32 y, bool raft = false, bool holiday = false);
    void animateBackground();
    void displaySplash();

    size_t setPosToLabel(std::pair<u16, u16> lastPlayed, size_t next = 0);
    void addTTM(Command cmd);
    void runTTMs();

public:
    Robinson(const std::string &ResMap, const std::string &ScrExe, bool readUnpacked);
    ~Robinson();

    bool navigateMenu(SDL_Keycode key);

    void initMenu(TTF_Font *font);
    void initRenderer(SDL_Renderer *rendererSDL);

    void advanceADSScript(std::pair<u16, u16> lastPlayed = std::make_pair((u16)0,(u16)0));
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

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
protected:
    //menu stuff
    std::map<std::string, SDL_Texture *> menuScreen;
    std::map<std::string, std::map<u_int16_t, SDL_Rect> > menuPos;
    std::string currentMenuScreen;
    size_t currentMenuPos;
    bool renderMenu;

    RESFile *res;
    ADSFile *ads;
    std::vector<Command> script;
    size_t scriptPos;
    std::multimap<std::pair<u_int16_t, u_int16_t>, size_t> labels;

    RIFFPlayer *audioPlayer;

    SDL_Renderer *renderer;
    SDL_Texture *rendererTarget;

    SDL_Color *palette;

    int8_t animationCycle;
    int8_t islandPos;
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

    std::list<std::pair<u_int16_t, u_int16_t> > lastTTMs;

    // lots of rects...
    SDL_Rect fullRect;
    SDL_Rect oceanRect;
    SDL_Rect screenRect;

    //u_int16_t currentMovie;
    std::string name;

    std::list<TTMPlayer *> ttmScenes;

    BMPFile *images[MAX_IMAGES];

    u_int16_t delay;
    u_int16_t currentMovie;
    u_int16_t queuedMovie;
    size_t queuedPos;

    u_int32_t delayTicks;

    void resetPlayer();

    void menuRenderer();
    void renderBackgroundAtPos(u_int16_t num, int32_t x, int32_t y, bool raft = false, bool holiday = false);
    void animateBackground();
    void displaySplash();

    size_t setPosToLabel(std::pair<u_int16_t, u_int16_t> lastPlayed, size_t next = 0);
    void addTTM(Command cmd);
    void runTTMs();

public:
    Robinson(std::string ResMap, std::string ScrExe);
    ~Robinson();

    bool navigateMenu(SDL_Keycode key);

    void initMenu(TTF_Font *font);
    void initRenderer(SDL_Renderer *rendererSDL);

    void advanceADSScript(std::pair<u_int16_t, u_int16_t> lastPlayed = std::make_pair((u_int16_t)0,(u_int16_t)0));
    void advanceScripts();
    void render();

    bool isMenuOpen() { return renderMenu; }
    bool isMovieRunning() { return movieRunning; }

    bool loadMovie(std::string adsName, u_int16_t num);
    void startMovie();

    u_int32_t getCurrentDelay() { return delay; }
    void displayMenu(bool show) { renderMenu = show; }
};

}

#endif // ROBINSON_H

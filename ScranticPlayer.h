#ifndef SCRANTICPLAYER_H
#define SCRANTICPLAYER_H

#include "RESFile.h"
#include "ADSFile.h"
#include "TTMFile.h"
#include "SCRFile.h"
#include "BMPFile.h"
#include "PALFile.h"

#include <list>

#ifdef WIN32
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#endif

#define MAX_IMG_SLOTS 10
#define MAX_AUDIO_SAMPLES 24
#define SPRITE_MIRROR 0x1

#define ISLAND_RIGHT 0x1
#define ISLAND_LEFT  0x2
#define NO_ISLAND    0x0

/*relative:
   0     0 trunk
 -77   -26 top
-154   131 island
-172   158 wave left
 -78   171 wave middle
  78   155 wave right
-193   155 large island
-292   180 stone
-313   192 wave stone
-209   266 large wave left
 -75   208 large wave middle (might be off)
 116   175 large wave right
  73   118 raft
 -46   131 top shadow*/

#define TOP_X -77
#define TOP_Y -26
#define ISLAND_X -154
#define ISLAND_Y 131
#define WAVE_LEFT_X -172
#define WAVE_LEFT_Y 158
#define WAVE_MID_X -78
#define WAVE_MID_Y 171
#define WAVE_RIGHT_X 78
#define WAVE_RIGHT_Y 155
#define STONE_X -292
#define STONE_Y 180
#define WAVE_STONE_X -313
#define WAVE_STONE_Y 192
#define L_ISLAND_X -193
#define L_ISLAND_Y 155
#define WAVE_L_LEFT_X -209
#define WAVE_L_LEFT_Y 175
#define WAVE_L_MID_X -75
#define WAVE_L_MID_Y 208
#define WAVE_L_RIGHT_X 116
#define WAVE_L_RIGHT_Y 175
#define RAFT_X 73
#define RAFT_Y 118
#define TOP_SHADOW_X -46
#define TOP_SHADOW_Y 131

#define ISLAND_RIGHT_X 443
#define ISLAND_RIGHT_Y 168

#define ISLAND_LEFT_X 171
#define ISLAND_LEFT_Y 178

#define ISLAND_TEMP_X 442
#define ISLAND_TEMP_Y 148

#define ISLAND2_X 170
#define ISLAND2_Y 148

#define SPRITE_ISLAND 0
#define SPRITE_L_ISLAND 1
#define SPRITE_STONE 2
#define SPRITE_WAVE_LEFT 3
#define SPRITE_WAVE_MID 6
#define SPRITE_WAVE_RIGHT 9
#define SPRITE_TOP 12
#define SPRITE_TRUNK 13
#define SPRITE_TOP_SHADOW 14
#define SPRITE_WAVE_L_LEFT 30
#define SPRITE_WAVE_L_MID 33
#define SPRITE_WAVE_L_RIGHT 36
#define SPRITE_WAVE_STONE 39

#define RITEM_SPRITE 0
#define RITEM_LINE 1
#define RITEM_RECT 2
#define RITEM_ELLIPSE 3

namespace SCRANTIC {

struct RenderItem
{
    SDL_Texture *tex;
    SDL_Rect src;
    SDL_Rect dest;
    u_int16_t num;
    u_int8_t flags;
    std::pair<u_int16_t, u_int16_t> color;
    u_int8_t itemType;
};

/*struct Sprite
{
    SDL_Texture *tex;
    SDL_Rect src;
    SDL_Rect dest;
    u_int16_t num;
    u_int8_t flags;
};

struct Line
{
    u_int16_t x1;
    u_int16_t y1;
    u_int16_t x2;
    u_int16_t y2;
    std::pair<u_int16_t, u_int16_t> color;
};

struct Rectangle
{
    SDL_Rect rect;
    std::pair<u_int16_t, u_int16_t> color;
};*/

struct TTMScene
{
    std::string ttm;
    u_int16_t resNo;
    u_int16_t scene;
    u_int16_t repeat;
    u_int16_t blob;
    bool wantsInit;
};

struct IslandState
{
    u_int8_t islandPos;
    bool night;
    bool largerIsland;
    u_int16_t trunk_x;
    u_int16_t trunk_y;
};

struct RendererState
{
    bool ttmSceneRunning;
    bool saveImage;
    bool saveNewImage;
    bool displayMenu;
    bool clear;

    SDL_Renderer *renderer;

    // need to be freed
    SDL_Texture *bg;
    SDL_Texture *fg;
    SDL_Texture *savedImage;

    //freed by SCRFile
    SDL_Texture *currentOcean;
    SDL_Texture *nightOcean;
    SDL_Texture *currentScreen;

    // window surface texture
    SDL_Texture *rendererTarget;

    //freed by BMPFile
    BMPFile *images[MAX_IMG_SLOTS];
    BMPFile *background;
    BMPFile *holiday;
    BMPFile *raft;

    // lots of rects...
    SDL_Rect bgRect;
    SDL_Rect fgRect;
    SDL_Rect savRect;
    SDL_Rect saveRect;
    SDL_Rect oceanRect;
    SDL_Rect screenRect;
    SDL_Rect clipRegion;

    SDL_Color *palette;
    std::pair<u_int16_t, u_int16_t> color;

    // stuff to draw
    std::list<RenderItem> renderItems;
    std::list<RenderItem> queuedItems;

    // wave animation counter
    int8_t animationCycle;

    u_int8_t currentImgSlot;
    u_int8_t currentPalSlot; // unused

    // menu textures
    std::map<std::string, SDL_Texture *> menuScreen;
    std::map<std::string, std::map<u_int16_t, SDL_Rect> > menuPos;
    std::string currentMenuScreen;
    size_t currentMenuPos;
};

class ScranticPlayer
{
protected:
    bool TTMInit;
    bool isRandom;
    bool newScene;
    bool lastPlayed;
    bool currentMovieFinished;

    RESFile *res;
    ADSFile *currentADS;
    TTMFile *currentTTM;

    u_int32_t delay;

    IslandState i_state;
    RendererState r_state;

    TTMScene currentScene;
    u_int16_t currentMovie;
    int32_t queuedMovie;
    int32_t lastMovie;

    std::vector<TTMScene> pickRandom;
    std::list<TTMScene> playList;
    std::vector<u_int8_t> rawAudio[MAX_AUDIO_SAMPLES];
    Mix_Chunk *audioSamples[MAX_AUDIO_SAMPLES];

    void purgePlayer();
    void resetPlayer();
    void renderBackgroundAtPos(SDL_Renderer *renderer, u_int16_t num, int32_t x, int32_t y, bool raft = false, bool holiday = false);
    void animateBackground(SDL_Renderer *renderer);
    void renderMenu();
    void StartTTMScene();

public:
    ScranticPlayer(std::string ResMap, std::string ScrExe);
    void AdvanceScript();
    void render();
    void initRenderer(SDL_Renderer *renderer, TTF_Font *font);
    void LoadADS(std::string ads, int32_t num = -1);
    void StartADSMovie(u_int16_t num);
    void displayMenu(bool show) { r_state.displayMenu = show; }
    bool isMenuOpen() { return r_state.displayMenu; }
    bool navigateMenu(SDL_Keycode key);
    bool isADSMovieFinished() { return currentMovieFinished; }
    u_int32_t getCurrentDelay() { return delay; }
    ~ScranticPlayer();
};

}

#endif // SCRANTICPLAYER_H

#include <iostream>

#include "RESFile.h"
#include "SCRFile.h"
#include "PALFile.h"
#include "BMPFile.h"
#include "Robinson.h"

#ifdef WIN32
#include <SDL.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#endif

#define ticksPerFrame 1000/30

using namespace std;

SDL_Window *g_Mainwindow = NULL;
SDL_Renderer *g_Renderer = NULL;
TTF_Font *g_Font = NULL;

void cleanup() {
    if (g_Renderer != NULL) {
        SDL_DestroyRenderer(g_Renderer);
    }

    if (g_Mainwindow != NULL) {
        SDL_DestroyWindow(g_Mainwindow);
    }

    TTF_CloseFont(g_Font);

    Mix_Quit();
    TTF_Quit();
    SDL_Quit();
}

bool init() {
    //Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize VIDEO! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (TTF_Init() < 0) {
        std::cerr << "SDL could not initialize TTF! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    g_Font = TTF_OpenFont("font.ttf", 14);

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize AUDIO! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (Mix_OpenAudio(11025, AUDIO_U8, 1, 2048) < 0) {
        printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
        return false;
    }

    //Create window
    g_Mainwindow = SDL_CreateWindow(
                       "Johnny's World",
                       SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED,
                       640,
                       480,
                       SDL_WINDOW_SHOWN
                   );

    if (g_Mainwindow == NULL) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        cleanup();
        return false;
    }

    g_Renderer = SDL_CreateRenderer(g_Mainwindow, -1, 0);
    if (g_Renderer == NULL) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        cleanup();
        return false;
    }

    // Set size of renderer to the same as window
    SDL_RenderSetLogicalSize(g_Renderer, 640, 480);

    return true;
}

int main() {
    if (!init()) {
        std::cerr << "Error in init()!" << std::endl;
        return 1;
    }

    cout << "Hello Johnny's World!" << endl;
    SCRANTIC::Robinson *crusoe = new SCRANTIC::Robinson("RESOURCE.MAP", "SCRANTIC.SCR");

    crusoe->initRenderer(g_Renderer);
    crusoe->initMenu(g_Font);

    crusoe->loadMovie("VISITOR.ADS", 3);
    crusoe->startMovie();

    bool quit = false;
    SDL_Event e;
    u16 delay;
    Uint32 ticks, newTicks, frameTicks, waitTicks;
    ticks = SDL_GetTicks();
    bool waiting = false;
    bool pause = false;

    while (!quit) {
        newTicks = SDL_GetTicks();
        frameTicks = newTicks - ticks;

        if (!waiting) {
            crusoe->advanceScripts();
            if (!crusoe->isMovieRunning()) {
                crusoe->displayMenu(true);
            }
            delay = crusoe->getCurrentDelay();
            if (delay == 0) {
                delay = 100;
            }
        }

        if (!pause) {
            if (delay > frameTicks) {
                delay -= frameTicks;
            } else {
                delay = 0;
            }
        }

        if (frameTicks < ticksPerFrame) {
            waitTicks = ticksPerFrame - frameTicks;
        } else {
            waitTicks = 0;
        }

        ticks = newTicks;
        waiting = (delay > waitTicks);
        SDL_Delay(waitTicks);
        crusoe->render();
        SDL_RenderPresent(g_Renderer);

        while(SDL_PollEvent(&e) != 0) {
            switch(e.type) {
            case SDL_KEYDOWN:
                //case SDL_KEYUP:
                if (e.key.keysym.sym == SDLK_SPACE && !e.key.repeat) {
                    pause = !pause;
                } else if (e.key.keysym.sym == SDLK_ESCAPE && !e.key.repeat) {
                    if (crusoe->isMenuOpen())  {
                        crusoe->displayMenu(false);
                        pause = false;
                    } else {
                        quit = true;
                    }
                } else if (!e.key.repeat && crusoe->isMenuOpen())  {
                    if (crusoe->navigateMenu(e.key.keysym.sym)) {
                        pause = false;
                    }
                } else if (e.key.keysym.sym == SDLK_RETURN && !e.key.repeat) {
                    pause = true;
                    crusoe->displayMenu(true);
                }
                break;
            case SDL_QUIT:
                quit = true;
                break;
            }
        }
    }

    delete crusoe;
    cleanup();

    return 0;
}


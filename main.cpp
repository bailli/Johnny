#include <iostream>

#include "RESFile.h"
#include "SCRFile.h"
#include "PALFile.h"
#include "BMPFile.h"
#include "Robinson.h"

#include "CommandlineParser.h"
#include "RIFFPlayer.h"

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
std::string g_path = "";
bool g_readUnpackedRes = false;

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
                       WINDOW_WIDTH,
                       WINDOW_HEIGHT,
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
    SDL_RenderSetLogicalSize(g_Renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    return true;
}

bool handleCommandline(char **begin, char **end) {

    CommandlineParser commandlineParser(begin, end);

    std::string path = "";
    std::string outputPath = "";
    std::string prepackedPath = "";

    if (commandlineParser.cmdOptionExists("--resourcePath")) {
        path = commandlineParser.getCmdOption("--resourcePath");
        if (path.substr(path.size() - 1) != "/") {
            path += "/";
        }
        g_path = path;
    }

    if (commandlineParser.cmdOptionExists("--outputPath")) {
        outputPath = commandlineParser.getCmdOption("--outputPath");
        if (outputPath.substr(outputPath.size() - 1) != "/") {
            outputPath += "/";
        }
    }

    if (commandlineParser.cmdOptionExists("--prepackedPath")) {
        prepackedPath = commandlineParser.getCmdOption("--prepackedPath");
        if (prepackedPath.substr(prepackedPath.size() - 1) != "/") {
            prepackedPath += "/";
        }
    }

    if (commandlineParser.cmdOptionExists("--unpack")) {
        bool onlyFiles = commandlineParser.cmdOptionExists("--onlyFiles");
        SCRANTIC::RESFile res(path + "RESOURCE.MAP", false);
        res.unpackResources(outputPath, onlyFiles);
        SCRANTIC::RIFFPlayer::extractRIFFFiles(path + "SCRANTIC.SCR", outputPath + "RIFF/", true);
        return true;
    }

    if (commandlineParser.cmdOptionExists("--repack")) {
        SCRANTIC::RESFile res(path + "RESOURCE.MAP", true);
        res.repackResources(outputPath, prepackedPath);
        return true;
    }

    if (commandlineParser.cmdOptionExists("--unpackedResources")) {
        std::cout << "Unpacked resources will be read from " << path << std::endl;
        g_readUnpackedRes = true;
    }

    return false;
}

int main(int argc, char **argv) {
    if (handleCommandline(argv, argv + argc)) {
        return 0;
    }

    if (!init()) {
        std::cerr << "Error in init()!" << std::endl;
        return 1;
    }

    cout << "Hello Johnny's World!" << endl;
    SCRANTIC::Robinson *crusoe = new SCRANTIC::Robinson(g_path, g_readUnpackedRes);

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


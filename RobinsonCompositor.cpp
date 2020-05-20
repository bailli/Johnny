#include "RobinsonCompositor.h"
#include "defines.h"

SCRANTIC::RobinsonCompositor::RobinsonCompositor(SDL_Renderer* renderer, int width, int height, RESFile* resources) :
    renderer(renderer),
    width(width),
    height(height),
    resources(resources),
    fullScreenRect({0, 0, width, height}),
    animationCycle(0),
    islandPos(NO_ISLAND),
    islandTrunk({ ISLAND_TEMP_X, ISLAND_TEMP_Y }),
    absoluteIslandTrunkPos({ 0, 0 }),
    isNight(false),
    isLargeIsland(false),
    screenSCR(NULL) {

    backgroundBMP = static_cast<BMPFile *>(resources->getResource("BACKGRND.BMP"));
    raftBMP = static_cast<BMPFile *>(resources->getResource("MRAFT.BMP"));
    holidayBMP = static_cast<BMPFile *>(resources->getResource("HOLIDAY.BMP"));
    cloudsBMP = static_cast<BMPFile *>(resources->getResource("CLOUDS.BMP"));

    rendererTarget = SDL_GetRenderTarget(renderer);

    oceanSCRList = {
        static_cast<SCRFile *>(resources->getResource("OCEAN00.SCR")),
        static_cast<SCRFile *>(resources->getResource("OCEAN01.SCR")),
        static_cast<SCRFile *>(resources->getResource("OCEAN02.SCR")),
    };

    oceanNightSCR = static_cast<SCRFile *>(resources->getResource("NIGHT.SCR"));
    oceanSCR = oceanSCRList[0];

    splashSCR = static_cast<SCRFile *>(resources->getResource("INTRO.SCR"));

    // create background and foreground texture
    // better pixel format?
    oceanTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    bgTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    fgTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    saveTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
    SDL_SetTextureBlendMode(bgTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(fgTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(saveTexture, SDL_BLENDMODE_BLEND);
}

SCRANTIC::RobinsonCompositor::~RobinsonCompositor() {
    SDL_DestroyTexture(oceanTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(fgTexture);
    SDL_DestroyTexture(saveTexture);
}

void SCRANTIC::RobinsonCompositor::displaySplash() {
    renderFullScreenSCR(splashSCR);
    SDL_RenderPresent(renderer);
    SDL_Delay(2000);
}

void SCRANTIC::RobinsonCompositor::renderFullScreenSCR(SCRFile* scr) {
    SDL_Rect src;
    SDL_Texture *tex = scr->getImage(renderer, src);
    SDL_Rect dst = { 0, 0, src.w, src.h };
    SDL_RenderCopy(renderer, tex, &src, &dst);
}

void SCRANTIC::RobinsonCompositor::renderSpriteNumAtPos(BMPFile *bmp, u16 num, i32 x, i32 y) {
    SDL_Rect src, dest;
    SDL_Texture *tex = bmp->getImage(renderer, num, src);
    dest = { x, y, src.w, src.h };
    SDL_RenderCopy(renderer, tex, &src, &dest);
}

void SCRANTIC::RobinsonCompositor::animateBackground() {
    if (islandPos == NO_ISLAND) {
        return;
    }

    i8 spriteOffset = animationCycle/12;
    int trunkX = absoluteIslandTrunkPos.x != 0 ? absoluteIslandTrunkPos.x : islandTrunk.x;
    int trunkY = absoluteIslandTrunkPos.y != 0 ? absoluteIslandTrunkPos.y : islandTrunk.y;

    if (isLargeIsland) {
        renderSpriteNumAtPos(backgroundBMP, (SPRITE_WAVE_L_LEFT + spriteOffset), WAVE_L_LEFT_X + trunkX, WAVE_L_LEFT_Y + trunkY);
        renderSpriteNumAtPos(backgroundBMP, (SPRITE_WAVE_L_MID + ((spriteOffset + 1) % 3)), WAVE_L_MID_X + trunkX, WAVE_L_MID_Y + trunkY);
        renderSpriteNumAtPos(backgroundBMP, (SPRITE_WAVE_L_RIGHT + ((spriteOffset + 2) % 3)), WAVE_L_RIGHT_X + trunkX, WAVE_L_RIGHT_Y + trunkY);
        renderSpriteNumAtPos(backgroundBMP, (SPRITE_WAVE_STONE + spriteOffset), WAVE_STONE_X + trunkX, WAVE_STONE_Y + trunkY);
    } else {
        renderSpriteNumAtPos(backgroundBMP, (SPRITE_WAVE_LEFT + spriteOffset), WAVE_LEFT_X + trunkX, WAVE_LEFT_Y + trunkY);
        renderSpriteNumAtPos(backgroundBMP, (SPRITE_WAVE_MID + ((spriteOffset + 1) % 3)), WAVE_MID_X + trunkX, WAVE_MID_Y + trunkY);
        renderSpriteNumAtPos(backgroundBMP, (SPRITE_WAVE_RIGHT + ((spriteOffset + 2) % 3)), WAVE_RIGHT_X + trunkX, WAVE_RIGHT_Y + trunkY);
    }

    //weather is missing

    ++animationCycle;
    if (animationCycle >= 36) {
        animationCycle = 0;
    }
}

void SCRANTIC::RobinsonCompositor::render(std::list<TTMPlayer *>::iterator begin, std::list<TTMPlayer *>::iterator end) {
    // first render background
    SDL_SetRenderTarget(renderer, bgTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    SDL_SetRenderTarget(renderer, oceanTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (islandPos != NO_ISLAND) {
        if (isNight) {
            renderFullScreenSCR(oceanNightSCR);
        } else {
            renderFullScreenSCR(oceanSCR);
        }

        int trunkX = absoluteIslandTrunkPos.x != 0 ? absoluteIslandTrunkPos.x : islandTrunk.x;
        int trunkY = absoluteIslandTrunkPos.y != 0 ? absoluteIslandTrunkPos.y : islandTrunk.y;

        SDL_SetRenderTarget(renderer, bgTexture);
        if (isLargeIsland) {
            renderSpriteNumAtPos(backgroundBMP, SPRITE_L_ISLAND, L_ISLAND_X + trunkX, L_ISLAND_Y + trunkY);
            renderSpriteNumAtPos(backgroundBMP, SPRITE_STONE, STONE_X + trunkX, STONE_Y + trunkY);
        }

        renderSpriteNumAtPos(backgroundBMP, SPRITE_ISLAND, ISLAND_X + trunkX, ISLAND_Y + trunkY);
        renderSpriteNumAtPos(backgroundBMP, SPRITE_TOP_SHADOW, TOP_SHADOW_X + trunkX, TOP_SHADOW_Y + trunkY);
        renderSpriteNumAtPos(backgroundBMP, SPRITE_TRUNK, trunkX, trunkY);
        renderSpriteNumAtPos(backgroundBMP, SPRITE_TOP, TOP_X + trunkX, TOP_Y + trunkY);
        renderSpriteNumAtPos(raftBMP, 0, RAFT_X + trunkX, RAFT_Y + trunkY);

        animateBackground();
    } else {
        if (screenSCR != NULL) {
            renderFullScreenSCR(screenSCR);
        }
    }

    int shiftX = absoluteIslandTrunkPos.x != 0 ? absoluteIslandTrunkPos.x - islandTrunk.x : 0;
    int shiftY = absoluteIslandTrunkPos.y != 0 ? absoluteIslandTrunkPos.y - islandTrunk.y : 0;
    SDL_Rect targetRect;
    u8 save;
    std::string scrName;

    // saved image
    SDL_SetRenderTarget(renderer, saveTexture);
    for (auto it = begin; it != end; ++it) {
        save = (*it)->needsSave();
        if (save != SAVE_NOSAVE) {
            if (save == SAVE_NEWIMAGE) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);
            }
            SDL_RenderCopy(renderer, (*it)->savedImage, &fullScreenRect, &fullScreenRect);
        }

        //pre render foreground
        (*it)->renderForeground(shiftX, shiftY);
    }
    // background end

    // render everything to screen
    SDL_SetRenderTarget(renderer, rendererTarget);
    SDL_RenderClear(renderer);

    targetRect = { 0, 0, width, height};
    shiftRect(&targetRect, shiftX, shiftY);

    SDL_RenderCopy(renderer, oceanTexture, &fullScreenRect, &fullScreenRect);
    SDL_RenderCopy(renderer, bgTexture, &fullScreenRect, &fullScreenRect);
    SDL_RenderCopy(renderer, saveTexture, &fullScreenRect, &fullScreenRect);

    for (auto it = begin; it != end; ++it) {
        if ((*it)->isClipped()) {
            SDL_Rect clipRect = (*it)->getClipRect();
            shiftRect(&clipRect, shiftX, shiftY);
            SDL_RenderCopy(renderer, (*it)->fg, &clipRect, &clipRect);
        } else {
            SDL_RenderCopy(renderer, (*it)->fg, &fullScreenRect, &fullScreenRect);
        }

        scrName = (*it)->getSCRName();
        if (scrName != "") {
            setScreen(scrName);
        }
    }
}

void SCRANTIC::RobinsonCompositor::reset() {
    islandPos = NO_ISLAND;
    screenSCR = NULL;

    absoluteIslandTrunkPos = { 0, 0 };

    SDL_SetRenderTarget(renderer, saveTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, rendererTarget);
}

void SCRANTIC::RobinsonCompositor::setScreen(const std::string &screen) {
    screenSCR = static_cast<SCRFile *>(resources->getResource(screen));
    if (screen == "ISLETEMP.SCR") {
        islandPos = ISLAND_RIGHT;
        islandTrunk.x = ISLAND_TEMP_X;
        islandTrunk.y = ISLAND_TEMP_Y;
    } else if (screen == "ISLAND2.SCR") {
        islandPos = ISLAND_LEFT;
        islandTrunk.x = ISLAND2_X;
        islandTrunk.y = ISLAND2_Y;
    } else {
        islandPos = NO_ISLAND;
    }
}

void SCRANTIC::RobinsonCompositor::shiftRect(SDL_Rect* rect, int x, int y) {
    rect->x += x;
    rect->y += y;
    if (rect->x < 0) {
        rect->w += rect->x;
        rect->x = 0;
    }
    if (rect->y < 0) {
        rect->h += rect->y;
        rect->y = 0;
    }
    if (rect->x + rect->w > width) {
        rect->w = width - rect->x;
    }
    if (rect->y + rect->h > height) {
        rect->h = height - rect->y;
    }
}

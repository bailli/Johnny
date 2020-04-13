#include "TTMPlayer.h"

#ifdef WIN32
#include <SDL2_gfxPrimitives.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif

SCRANTIC::TTMPlayer::TTMPlayer(const std::string &ttmName, u16 resNum, u16 scene, i16 repeatCount, RESFile *resFile, BMPFile **BMPs, SDL_Color *pal, SDL_Renderer *rendererContext)
    : resNo(resNum),
      sceneNo(scene),
      originalScene(scene),
      repeat(repeatCount),
      delay(0),
      remainingDelay(0),
      waitCount(0),
      waitDelay(0),
      imgSlot(0),
      audioSample(-1),
      jumpToScript(-1),
      renderer(rendererContext),
      clipRegion(false),
      alreadySaved(true),
      saveNewImage(false),
      palette(pal),
      maxTicks(0),
      selfDestruct(false),
      selfDestructActive(false),
      saveImage(false),
      isDone(false),
      toBeKilled(false),
      images(BMPs),
      res(resFile),
      screen(""),
      ttm(NULL),
      savedImage(NULL),
      fg(NULL) {

    for (int i = 0; i < MAX_IMAGES; ++i) {
        oldImages[i] = images[i];
    }

    ttm = static_cast<TTMFile *>(res->getResource(ttmName));

    if (!ttm) {
        return;
    }

    script = ttm->getFullScene(scene);
    if (script.size()) {
        scriptPos = script.begin();
    }

    name = ttm->filename + " - " + ttm->getTag(scene);

    savedImage = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    fg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetTextureBlendMode(savedImage, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(fg, SDL_BLENDMODE_BLEND);

    saveRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    if (repeat < 0) {
        maxTicks = -DELAY_MS*repeat;
        repeat = 0;
        selfDestructActive = true;
    }

}

SCRANTIC::TTMPlayer::~TTMPlayer() {
    SDL_DestroyTexture(savedImage);
    SDL_DestroyTexture(fg);
}

u16 SCRANTIC::TTMPlayer::getRemainigDelay(u32 ticks) {
    if (selfDestructActive) {
        maxTicks -= ticks;

        if (maxTicks < 1) {
            selfDestruct = true;
            std::cout << "Scene self-destructing! " << sceneNo << std::endl;
        }
    }

    if (ticks < remainingDelay) {
        remainingDelay -= ticks;
    } else {
        remainingDelay = 0;
    }

    return remainingDelay;
}

void SCRANTIC::TTMPlayer::advanceScript() {
    if (toBeKilled) {
        isDone = true;
        return;
    }

    if (waitCount) {
        remainingDelay = waitDelay/DELAY_MS;
        waitCount--;
        return;
    }

    remainingDelay = delay;

    if ((jumpToScript >= 0) && !selfDestruct) {
        if (jumpToScript == sceneNo) {
            scriptPos = script.begin();
        } else {
            std::cout << "Jump to different sceneNo! From " << sceneNo << " to Scene " << jumpToScript << std::endl;
            sceneNo = jumpToScript;
            script.clear();
            script = ttm->getFullScene(sceneNo);
            if (script.size()) {
                scriptPos = script.begin();
            }

            name = ttm->filename + " - " + ttm->getTag(sceneNo);
        }

        jumpToScript = -1;
    }


    if (scriptPos == script.end()) {
        if (repeat) {
            --repeat;
            scriptPos = script.begin();
        } else {
            isDone = true;
            return;
        }
    }

    Command cmd;
    SceneItem item;

    bool stop = false;
    audioSample = -1;
    saveImage = false;
    saveNewImage = false;
    screen = "";

    for (; scriptPos != script.end(); ++scriptPos) {
        cmd = (*scriptPos);

        switch (cmd.opcode) {
        case CMD_PURGE:
            clipRegion = false;
            break;

        case CMD_UPDATE:
            items.splice(items.end(), queuedItems);
            stop = true;
            break;

        case CMD_DELAY:
            delay = cmd.data.at(0) * DELAY_MS;
            remainingDelay = delay;
            break;

        case CMD_SEL_SLOT_IMG:
            imgSlot = cmd.data.at(0);
            break;

        case CMD_SEL_SLOT_PAL:
            // ignored - there is only one palette...
            //palSlot = cmd.data.at(0);
            break;

        case CMD_CLEAR_IMGSLOT:
            images[imgSlot] = oldImages[imgSlot];
            break;

        case CMD_SET_SCENE:
        case CMD_SET_SCENE_LABEL:
            std::cout << "TTM Scene: " << cmd.name << std::endl;
            break;

        case CMD_JMP_SCENE:
            jumpToScript = cmd.data.at(0);
            std::cout << "TTM Command: jump to script " << cmd.data.at(0) << std::endl;
            break;

        case CMD_TIMER:
            waitCount = cmd.data.at(0);
            waitDelay = cmd.data.at(1);
            break;

        case CMD_CLIP_REGION:
            clipRect.x = (i16)cmd.data.at(0);
            clipRect.y = (i16)cmd.data.at(1);
            clipRect.w = cmd.data.at(2) - clipRect.x;
            clipRect.h = cmd.data.at(3) - clipRect.y;
            if (clipRect.x + clipRect.w >= SCREEN_WIDTH) {
                clipRect.w = SCREEN_WIDTH - clipRect.x;
            }
            if (clipRect.y + clipRect.h >= SCREEN_HEIGHT) {
                clipRect.h = SCREEN_HEIGHT - clipRect.y;
            }
            clipRegion = true;
            break;

        case CMD_SAVE_IMAGE_NEW:
            saveNewImage = true;
        case CMD_SAVE_IMAGE:
            alreadySaved = false;
            saveImage = true;
            saveRect.x = (i16)cmd.data.at(0);
            saveRect.y = (i16)cmd.data.at(1);
            saveRect.w = cmd.data.at(2);
            saveRect.h = cmd.data.at(3);
            items.splice(items.end(), queuedItems);
            renderForeground();
            //stop = true;
            break;

        case CMD_DRAW_PIXEL:
            item.src.x = (i16)cmd.data.at(0);
            item.src.y = (i16)cmd.data.at(1);
            item.src.w = 2;
            item.src.h = 2;
            item.color = currentColor;
            item.itemType = RENDERITEM_RECT;
            queuedItems.push_back(item);
            break;

        case CMD_DRAW_LINE:
            item.src.x = (i16)cmd.data.at(0);
            item.src.y = (i16)cmd.data.at(1);
            item.src.w = (i16)cmd.data.at(2);
            item.src.h = (i16)cmd.data.at(3);
            item.color = currentColor;
            item.itemType = RENDERITEM_LINE;
            queuedItems.push_back(item);
            break;

        case CMD_SET_COLOR:
            currentColor = std::make_pair(cmd.data.at(0), cmd.data.at(1));
            break;

        case CMD_DRAW_RECTANGLE:
            item.src.x = (i16)cmd.data.at(0);
            item.src.y = (i16)cmd.data.at(1);
            item.src.w = cmd.data.at(2);
            item.src.h = cmd.data.at(3);
            item.color = currentColor;
            item.itemType = RENDERITEM_RECT;
            queuedItems.push_back(item);
            break;
        case CMD_DRAW_ELLIPSE:
            item.src.w = cmd.data.at(2)/2;
            item.src.h = cmd.data.at(3)/2;
            item.src.x = (i16)cmd.data.at(0) + item.src.w;
            item.src.y = (i16)cmd.data.at(1) + item.src.h;
            item.color = currentColor;
            item.itemType = RENDERITEM_ELLIPSE;
            queuedItems.push_back(item);
            break;

        case CMD_DRAW_SPRITE_MIRROR:
        case CMD_DRAW_SPRITE:
            if (images[cmd.data.at(3)] != NULL) {
                item.dest.x = (i16)cmd.data.at(0);
                item.dest.y = (i16)cmd.data.at(1);
                item.num = cmd.data.at(2);
                item.tex = images[cmd.data.at(3)]->getImage(renderer, item.num, item.src);
                if (item.tex == NULL) {
                    std::cerr << name << ": Error tried to access non existing sprite number!" << std::endl;
                    std::cout << ">>> TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
                    break;
                }
                item.dest.w = item.src.w;
                item.dest.h = item.src.h;
                item.flags = 0;
                if (cmd.opcode == CMD_DRAW_SPRITE_MIRROR) {
                    item.flags |= RENDERFLAG_MIRROR;
                }
                item.itemType = RENDERITEM_SPRITE;
                queuedItems.push_back(item);
            } else {
                std::cerr << name << ": Error tried to access unloaded image slot!" << std::endl;
                std::cout << ">>> TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
            }
            break;

        case CMD_CLEAR_RENDERER:
            items.clear();
            break;

        case CMD_PLAY_SOUND:
            if ((cmd.data.at(0) < 1) || (cmd.data.at(0) > MAX_AUDIO)) {
                break;
            }
            audioSample = cmd.data.at(0) - 1;
            break;

        case CMD_LOAD_SCREEN:
            screen = cmd.name;
            items.clear();
            break;

        case CMD_LOAD_BITMAP:
            images[imgSlot] = static_cast<BMPFile *>(res->getResource(cmd.name));
            break;

        case CMD_LOAD_PALETTE:
            // ignored - there is only one palette...
            break;

        default:
            std::cout << "TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
            break;
        }

        if (stop) {
            break;
        }
    }

    if (scriptPos == script.end()) {
        if (repeat) {
            --repeat;
            scriptPos = script.begin();
            return;
        } else {
            if (jumpToScript == -1) {
                isDone = true;
            }
            return;
        }
    }

    ++scriptPos;

    return;
}


void SCRANTIC::TTMPlayer::renderForeground() {
    SDL_SetRenderTarget(renderer, fg);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    Uint32 c1, c2;

    for (auto item = items.begin(); item != items.end(); ++item) {
        switch ((*item).itemType) {
        case RENDERITEM_SPRITE:
            SDL_RenderCopyEx(renderer, (*item).tex, &(*item).src, &(*item).dest, 0, NULL, (SDL_RendererFlip)(*item).flags);
            break;

        case RENDERITEM_LINE:
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawLine(renderer, (*item).src.x, (*item).src.y, (*item).src.w, (*item).src.h);
            break;

        case RENDERITEM_RECT:
            SDL_SetRenderDrawColor(renderer, palette[(*item).color.first].r, palette[(*item).color.first].g, palette[(*item).color.first].b, 255);
            SDL_RenderDrawRect(renderer, &(*item).src);
            SDL_SetRenderDrawColor(renderer, palette[(*item).color.second].r, palette[(*item).color.second].g, palette[(*item).color.second].b, 255);
            SDL_RenderFillRect(renderer, &(*item).src);
            break;

        case RENDERITEM_ELLIPSE:
            c1 = palette[(*item).color.first].r * 0x10000
                 + palette[(*item).color.first].g * 0x100
                 + palette[(*item).color.first].b + 0xFF000000;
            c2 = palette[(*item).color.second].r * 0x10000
                 + palette[(*item).color.second].g * 0x100
                 + palette[(*item).color.second].b + 0xFF000000;
            filledEllipseColor(renderer, (*item).src.x, (*item).src.y, (*item).src.w, (*item).src.h, c2);
            ellipseColor(renderer, (*item).src.x, (*item).src.y, (*item).src.w, (*item).src.h, c1);
            break;

        default:
            std::cerr << "ERROR: Renderer: Unkown render item type!" << std::endl;
            break;
        }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // save foreground rect
    if (!alreadySaved) {
        if (saveImage) {
            SDL_SetRenderTarget(renderer, savedImage);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, fg, &saveRect, &saveRect);
            alreadySaved = true;
        }
    }

    SDL_SetRenderTarget(renderer, fg);
}

u8 SCRANTIC::TTMPlayer::needsSave() {
    if (!saveImage) {
        return 0;
    } else if (saveNewImage) {
        return 2;
    } else {
        return 1;
    }
}

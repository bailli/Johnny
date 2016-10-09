#include "TTMPlayer.h"

#ifdef WIN32
#include <SDL2_gfxPrimitives.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif

SCRANTIC::TTMPlayer::TTMPlayer(std::string ttmName, u_int16_t resNum, u_int16_t scene, RESFile *resFile, BMPFile **BMPs, SDL_Color *pal, SDL_Renderer *rendererContext)
    : scriptPos(0), delay(20), clipRegion(false), renderer(rendererContext), res(resFile), savedImage(NULL), images(BMPs), palette(pal), alreadySaved(true)
{    
    TTMFile *ttm = static_cast<TTMFile *>(res->getResource(ttmName));

    if (!ttm)
        return;

    script = ttm->getFullScene(scene);
    if (script.size())
        scriptPos = script.begin();

    name = ttm->filename + " - " + ttm->getTag(scene);

    sceneNo = scene;
    resNo = resNum;

    savedImage = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 640, 480);
    fg = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 640, 480);
    SDL_SetTextureBlendMode(savedImage, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(fg, SDL_BLENDMODE_BLEND);

    saveRect.x = 0;
    saveRect.h = 0;
    saveRect.w = 640;
    saveRect.h = 480;
}

SCRANTIC::TTMPlayer::~TTMPlayer()
{
    SDL_DestroyTexture(savedImage);
    SDL_DestroyTexture(fg);
}

u_int16_t SCRANTIC::TTMPlayer::getDelay()
{
    return delay;
}

SCRANTIC::SceneItem SCRANTIC::TTMPlayer::getSceneItem(bool reset)
{
    if (reset)
        itemPos = items.begin();

    SceneItem i;

    if (itemPos == items.end())
    {
        i.itemType = RENDERITEM_NONE;
        return i;
    }

    i = (*itemPos);
    ++itemPos;

    return i;
}

u_int16_t SCRANTIC::TTMPlayer::advanceScript()
{
    if (scriptPos == script.end())
        return ttmFinished;

    Command cmd;
    SceneItem item;

    u_int16_t flag = 0;

    bool stop = false;

    for (; scriptPos != script.end(); ++scriptPos)
    {
        cmd = (*scriptPos);

        //std::cout << "TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;

        switch (cmd.opcode)
        {
        case CMD_PURGE:
            clipRegion = false;
            flag |= ttmPurge;
            break;

        case CMD_UPDATE:
            items.splice(items.end(), queuedItems);
            flag |= ttmUpdate;
            stop = true;
            break;

        case CMD_DELAY:
            delay = cmd.data.at(0) * 20;
            break;

        case CMD_SEL_SLOT_IMG:
            imgSlot = cmd.data.at(0);
            break;

        case CMD_SEL_SLOT_PAL:
            //palSlot = cmd.data.at(0);
            break;

        case CMD_SET_SCENE:
            std::cout << "TTM Scene: " << cmd.name << std::endl;
            break;
/*            case CMD_UNK_2020:
//                if (cmd.data.at(0) == 0x003C)
            {
                std::cout << "TTM Command: simple wait timer for " << (int32_t)cmd.data.at(1) << std::endl;
                SDL_Delay(cmd.data.at(1)*20);
            }
//                else
//                    std::cout << "TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
            break;*/

        case CMD_CLIP_REGION:
            clipRect.x = (int16_t)cmd.data.at(0);
            clipRect.y = (int16_t)cmd.data.at(1);
            clipRect.w = cmd.data.at(2) - clipRect.x;
            clipRect.h = cmd.data.at(3) - clipRect.y;
            if (clipRect.x + clipRect.w >= 640)
                clipRect.w = 640 - clipRect.x;
            if (clipRect.y + clipRect.h >= 480)
                clipRect.h = 480 - clipRect.y;
            clipRegion = true;
            break;

        case CMD_SAVE_IMAGE_NEW:
            flag |= ttmSaveNew;
        case CMD_SAVE_IMAGE:
            alreadySaved = false;
            saveRect.x = (int16_t)cmd.data.at(0);
            saveRect.y = (int16_t)cmd.data.at(1);
            saveRect.w = cmd.data.at(2);
            saveRect.h = cmd.data.at(3);
            flag |= ttmSaveImage;
            items.splice(items.end(), queuedItems);
            lastResult = flag;
            renderForeground();
            //stop = true;
            break;

        case CMD_DRAW_PIXEL:
            item.src.x = (int16_t)cmd.data.at(0);
            item.src.y = (int16_t)cmd.data.at(1);
            item.src.w = 2;
            item.src.h = 2;
            item.color = currentColor;
            item.itemType = RENDERITEM_RECT;
            queuedItems.push_back(item);
            break;

        case CMD_DRAW_LINE:
            item.src.x = (int16_t)cmd.data.at(0);
            item.src.y = (int16_t)cmd.data.at(1);
            item.src.w = (int16_t)cmd.data.at(2);
            item.src.h = (int16_t)cmd.data.at(3);
            item.color = currentColor;
            item.itemType = RENDERITEM_LINE;
            queuedItems.push_back(item);
            break;

        case CMD_SET_COLOR:
            currentColor = std::make_pair(cmd.data.at(0), cmd.data.at(1));
            break;

        case CMD_DRAW_RECTANGLE:
            item.src.x = (int16_t)cmd.data.at(0);
            item.src.y = (int16_t)cmd.data.at(1);
            item.src.w = cmd.data.at(2);
            item.src.h = cmd.data.at(3);
            item.color = currentColor;
            item.itemType = RENDERITEM_RECT;
            queuedItems.push_back(item);
            break;
        case CMD_DRAW_ELLIPSE:
            item.src.w = cmd.data.at(2)/2;
            item.src.h = cmd.data.at(3)/2;
            item.src.x = (int16_t)cmd.data.at(0) + item.src.w;
            item.src.y = (int16_t)cmd.data.at(1) + item.src.h;
            item.color = currentColor;
            item.itemType = RENDERITEM_ELLIPSE;
            queuedItems.push_back(item);
            break;

        case CMD_DRAW_SPRITE_MIRROR:
        case CMD_DRAW_SPRITE:
            if (images[cmd.data.at(3)] != NULL)
            {
                item.dest.x = (int16_t)cmd.data.at(0);
                item.dest.y = (int16_t)cmd.data.at(1);
                item.num = cmd.data.at(2);
                item.tex = images[cmd.data.at(3)]->getImage(renderer, item.num, item.src);
                if (item.tex == NULL)
                {
                    std::cerr << name << ": Error tried to access non existing sprite number!" << std::endl;
                    std::cout << ">>> TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
                    break;
                }
                item.dest.w = item.src.w;
                item.dest.h = item.src.h;
                item.flags = 0;
                if (cmd.opcode == CMD_DRAW_SPRITE_MIRROR)
                    item.flags |= RENDERFLAG_MIRROR;
                item.itemType = RENDERITEM_SPRITE;
                queuedItems.push_back(item);
            }
            else
            {
                std::cerr << name << ": Error tried to access unloaded image slot!" << std::endl;
                std::cout << ">>> TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
            }
            break;

        case CMD_CLEAR_RENDERER:
            items.clear();
            break;

        case CMD_PLAY_SOUND:
            if ((cmd.data.at(0) < 1) || (cmd.data.at(0) > MAX_AUDIO))
                break;
            flag |= ttmPlaySound;
            audioSample = cmd.data.at(0) - 1;
            break;

        case CMD_LOAD_SCREEN:
            screen = cmd.name;
            flag |= ttmLoadScreen;
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

        if (stop)
            break;
    }

    if (scriptPos == script.end())
        return ttmFinished;

    ++scriptPos;

    if (clipRegion)
        flag |= ttmClipRegion;

    lastResult = flag;

    return flag;
}


void SCRANTIC::TTMPlayer::renderForeground()
{
    SDL_SetRenderTarget(renderer, fg);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    Uint32 c1, c2;

    for (auto item = items.begin(); item != items.end(); ++item)
    {
        switch ((*item).itemType)
        {
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
    if (!alreadySaved)
        if (lastResult & ttmSaveImage)
        {
            SDL_SetRenderTarget(renderer, savedImage);
            //don't know why there is more stuff if this check is compiled...
            //if (lastResult & ttmSaveNew)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);
            }
            SDL_RenderCopy(renderer, fg, &saveRect, &saveRect);
            alreadySaved = true;
        }

    SDL_SetRenderTarget(renderer, fg);
}

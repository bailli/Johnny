#include "ScranticParallelPlayer.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

#ifdef WIN32
#include <SDL2_gfxPrimitives.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif

SCRANTIC::ScranticParallelPlayer::ScranticParallelPlayer(std::string ResMap, std::string ScrExe)
    : lookForLabel(false), labelRes(0), labelScene(0), currentMovieFinished(false), isRandom(false),
      res(NULL), currentADS(NULL), delay(1000), currentMovie(0), queuedMovie(-1), lastMovie(-1)
{
    r_state.menuPos.insert(std::make_pair(std::string(), std::map<u_int16_t, SDL_Rect>()));
    std::cerr << "Passed!!!" << std::endl;

    res = new RESFile(ResMap);

    PALFile *pal = static_cast<PALFile *>(res->getResource("JOHNCAST.PAL"));
    r_state.palette = pal->getPalette();
    for (auto it = res->resourceMap.begin(); it != res->resourceMap.end(); ++it)
        if (it->second.filetype == "BMP")
            static_cast<BMPFile *>(it->second.handle)->setPalette(pal->getPalette(), 256);
        else if (it->second.filetype == "SCR")
            static_cast<SCRFile *>(it->second.handle)->setPalette(pal->getPalette(), 256);

    for (int i = 0; i < MAX_AUDIO_SAMPLES; ++i)
        audioSamples[i] = NULL;

    std::srand(std::time(0));

    /*size_t offsets[] = { 0x1DC00, 0x20800, 0x20E00, 0x22C00, 0x24000, 0x24C00,
                         0x28A00, 0x2C600, 0x2D000, 0x2DE00, 0x32E00, 0x34400,
                         0x37200, 0x37E00, 0x39C00, 0x3AE00, 0x3E600, 0x3F400,
                         0x41200, 0x42600, 0x42C00, 0x43400, 0x45A00 };*/

    // not fully sorted yet?
    size_t offsets[] = { 0x1DC00, 0x20800, 0x20E00, 0x22C00, 0x24000, 0x24C00,
                         0x28A00, 0x2C600, 0x2D000, 0x2DE00, 0x34400, 0x32E00,
                         0x39C00, 0x43400, 0x37200, 0x37E00, 0x45A00, 0x3AE00,
                         0x3E600, 0x3F400, 0x41200, 0x42600, 0x42C00, 0x43400 };

    u_int32_t size;
    u_int8_t byte;
    SDL_RWops* rwops;

    std::ifstream in;
    in.open(ScrExe, std::ios::binary | std::ios::in);
    in.unsetf(std::ios::skipws);

    if (!in.is_open())
    {
        std::cerr << "ScranticPlayer: Error opening SCRANTIC.SCR" << std::endl;
        return;
    }

    for (u_int8_t i = 0; i < MAX_AUDIO_SAMPLES; ++i)
    {
        in.seekg(offsets[i]+4, std::ios_base::beg);
        SCRANTIC::BaseFile::u_read_le(&in, size);
        size += 8;

        rawAudio[i].reserve(size);
        in.seekg(offsets[i], std::ios_base::beg);

        for (u_int32_t j = 0; j < size; ++j)
        {
            SCRANTIC::BaseFile::u_read_le(&in, byte);
            rawAudio[i].push_back(byte);
        }

        //SCRANTIC::BaseFile::saveFile(rawAudio[i], "RIFF"+std::to_string(i)+".wav");

        rwops = SDL_RWFromMem((unsigned char*)&rawAudio[i].front(), rawAudio[i].size());
        audioSamples[i] = Mix_LoadWAV_RW(rwops, 1);
    }

    in.close();
}

SCRANTIC::ScranticParallelPlayer::~ScranticParallelPlayer()
{
    for (u_int8_t i = 0; i < MAX_AUDIO_SAMPLES; ++i)
        Mix_FreeChunk(audioSamples[i]);

    if (res != NULL)
        delete res;

    SDL_DestroyTexture(r_state.bg);
    //SDL_DestroyTexture(r_state.fg);
    //SDL_DestroyTexture(r_state.savedImage);

    /*for (auto scene = r_state.currentScenes.begin(); scene != r_state.currentScenes.end(); ++scene)
    {
        (*scene).queuedItems.clear();
        (*scene).renderItems.clear();
        SDL_DestroyTexture((*scene).savedImage);
        SDL_DestroyTexture((*scene).fg);
    }
    r_state.currentScenes.clear();*/

    for (auto tex : r_state.menuScreen)
        SDL_DestroyTexture(tex.second);
}

bool SCRANTIC::ScranticParallelPlayer::navigateMenu(SDL_Keycode key)
{
    size_t i;

    switch (key)
    {
    case SDLK_LEFT:
    case SDLK_RIGHT:
        for (i = 0; i < res->ADSFiles.size(); ++i)
            if (res->ADSFiles.at(i) == r_state.currentMenuScreen)
                break;

        if (i >= res->ADSFiles.size())
        {
            std::cerr << "Menu screen not found! " << r_state.currentMenuScreen << std::endl;
            return false;
        }

        if (key == SDLK_LEFT)
        {
            if (i == 0)
                i = res->ADSFiles.size()-1;
            else
                --i;
        }
        else
        {
            if (i == res->ADSFiles.size()-1)
                i = 0;
            else
                ++i;
        }

        r_state.currentMenuScreen = res->ADSFiles.at(i);
        r_state.currentMenuPos = r_state.menuPos[r_state.currentMenuScreen].begin()->first;
        break;
    case SDLK_RETURN:
        LoadADS(r_state.currentMenuScreen, r_state.currentMenuPos);
        r_state.displayMenu = false;
        return true;
    case SDLK_UP:
    case SDLK_DOWN:
        auto it = r_state.menuPos[r_state.currentMenuScreen].find(r_state.currentMenuPos);
        if (it == r_state.menuPos[r_state.currentMenuScreen].end())
        {
            std::cerr << "Menu position not found! " << r_state.currentMenuScreen << std::endl;
            return false;
        }

        if (key == SDLK_UP)
        {
            if (it == r_state.menuPos[r_state.currentMenuScreen].begin())
                it = r_state.menuPos[r_state.currentMenuScreen].end();
            --it;
        }
        else
        {
            ++it;
            if (it == r_state.menuPos[r_state.currentMenuScreen].end())
                it = r_state.menuPos[r_state.currentMenuScreen].begin();
        }
        r_state.currentMenuPos = it->first;
        break;
    }
    return false;
}

void SCRANTIC::ScranticParallelPlayer::renderMenu()
{
    SDL_Texture *tex;

    auto it = r_state.menuScreen.find(r_state.currentMenuScreen);
    if (it == r_state.menuScreen.end())
    {
        std::cerr << "Menu screen not found! " << r_state.currentMenuScreen << std::endl;
        return;
    }

    SDL_SetRenderDrawColor(r_state.renderer, 127, 127, 127, 255);
    SDL_RenderFillRect(r_state.renderer, &r_state.menuPos[r_state.currentMenuScreen][r_state.currentMenuPos]);
    SDL_RenderCopy(r_state.renderer, it->second, NULL, NULL);

    //r_state.displayMenu = false;
}

void SCRANTIC::ScranticParallelPlayer::initRenderer(SDL_Renderer *renderer, TTF_Font *font)
{
    r_state.renderer = renderer;

    // init island
    i_state.night = false;
    i_state.largerIsland = false;
    i_state.islandPos = NO_ISLAND;
    i_state.trunk_x = ISLAND_TEMP_X;
    i_state.trunk_y = ISLAND_TEMP_Y;

    // init wave animation cycle
    r_state.animationCycle = 0;

    // random pick ocean (0-2)
    r_state.oceanRect.x = 0;
    r_state.oceanRect.y = 0;
    u_int8_t random = std::rand() % 3;
    std::string ocean = "OCEAN0"+std::to_string(random)+".SCR";

    // load BMP files
    r_state.currentOcean = static_cast<SCRFile *>(res->getResource(ocean))->getImage(r_state.renderer, r_state.oceanRect);
    r_state.nightOcean = static_cast<SCRFile *>(res->getResource("NIGHT.SCR"))->getImage(r_state.renderer, r_state.oceanRect);
    r_state.background = static_cast<BMPFile *>(res->getResource("BACKGRND.BMP"));
    r_state.raft = static_cast<BMPFile *>(res->getResource("MRAFT.BMP"));
    r_state.holiday = static_cast<BMPFile *>(res->getResource("HOLIDAY.BMP"));

    //r_state.currentScreen = NULL;

    // many rects
    r_state.fullRect.x = 0;
    r_state.fullRect.y = 0;
    r_state.fullRect.w = 640;
    r_state.fullRect.h = 480;

    //r_state.clipRegion.x = 0;
    //r_state.clipRegion.y = 0;
    //r_state.clipRegion.w = 0;
    //r_state.clipRegion.h = 0;

    //r_state.screenRect.x = 0;
    //r_state.screenRect.y = 0;

    // create background and foreground texture
    // better pixel format?
    r_state.bg = SDL_CreateTexture(r_state.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_state.fullRect.w, r_state.fullRect.h);
    //r_state.fg = SDL_CreateTexture(r_state.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_state.fullRect.w, r_state.fullRect.h);
    //r_state.savedImage = SDL_CreateTexture(r_state.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_state.fullRect.w, r_state.fullRect.h);
    //SDL_SetTextureBlendMode(r_state.fg, SDL_BLENDMODE_BLEND);
    //SDL_SetTextureBlendMode(r_state.savedImage, SDL_BLENDMODE_BLEND);

    // remember actual window surface texture
    r_state.rendererTarget = SDL_GetRenderTarget(r_state.renderer);

    //r_state.saveImage = false;
    //r_state.saveNewImage = false;
    r_state.anyTTMSceneRunning = false;
    r_state.displayMenu = false;
    r_state.clear = false;

    //r_state.currentImgSlot = 0;
    //r_state.currentPalSlot = 0;

    //r_state.color = std::make_pair(0, 0);

    //generate menu textures
    ADSFile *ads;
    //std::vector<std::string> names;
    std::string name, adsstring;
    u_int16_t id;
    SDL_Rect rect;
    SDL_Surface *tmpSurface;
    SDL_Surface *menu;
    SDL_Texture *tex;
    SDL_Color c1, c2;

    c1.a = 255;
    c1.r = 255;
    c1.g = 0;
    c1.b = 0;

    c2.a = 255;
    c2.r = 255;
    c2.g = 255;
    c2.b = 255;

    rect.x = 20;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;

    for (size_t i = 0; i < res->ADSFiles.size(); ++i)
    {
        adsstring = res->ADSFiles.at(i);

        ads = static_cast<ADSFile *>(res->getResource(adsstring));

        menu = SDL_CreateRGBSurface(0, 640, 480, 32, 0, 0, 0, 0);

        rect.y = 10;

        TTF_SizeText(font, adsstring.c_str(), &rect.w, &rect.h);
        tmpSurface = TTF_RenderText_Blended(font, adsstring.c_str(), c1);

        if (tmpSurface == NULL)
            std::cerr << "ERROR: Renderer: Could not render text: " << name << std::endl;
        else
        {
            SDL_BlitSurface(tmpSurface, NULL, menu, &rect);
            SDL_FreeSurface(tmpSurface);
        }

        r_state.menuPos.insert(std::make_pair(adsstring, std::map<u_int16_t, SDL_Rect>()));

        for (auto tag : ads->tagList)
        {
            id = tag.first;
            name = tag.second;

            rect.y = rect.y + rect.h + 10;

            TTF_SizeText(font, name.c_str(), &rect.w, &rect.h);
            tmpSurface = TTF_RenderText_Blended(font, name.c_str(), c2);

            if (tmpSurface == NULL)
                std::cerr << "ERROR: Renderer: Could not render text: " << name << std::endl;
            else
            {
                SDL_BlitSurface(tmpSurface, NULL, menu, &rect);
                SDL_FreeSurface(tmpSurface);
                rect.w += 6;
                rect.x -= 3;
                rect.h += 6;
                rect.y -= 3;
                r_state.menuPos.at(adsstring).insert(std::make_pair(id, rect));
                rect.x += 3;
                rect.y += 3;
                rect.h -= 6;
            }

        }

        tex = SDL_CreateTextureFromSurface(r_state.renderer, menu);

        if (tex == NULL)
            std::cerr << "ERROR: Renderer: Could not convert menu surface to to texture: " << res->ADSFiles.at(i) << std::endl;
        else
        {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(tex, 200);
        }

        SDL_FreeSurface(menu);

        r_state.menuScreen.insert(std::make_pair(adsstring, tex));
    }
}

void SCRANTIC::ScranticParallelPlayer::animateBackground(SDL_Renderer *renderer)
{
    if (!i_state.islandPos)
        return;

    if (i_state.largerIsland)
    {
        renderBackgroundAtPos(renderer, (SPRITE_WAVE_L_LEFT + r_state.animationCycle/12), WAVE_L_LEFT_X + i_state.trunk_x, WAVE_L_LEFT_Y + i_state.trunk_y);
        renderBackgroundAtPos(renderer, (SPRITE_WAVE_L_MID + ((r_state.animationCycle/12 + 1) % 3)), WAVE_L_MID_X + i_state.trunk_x, WAVE_L_MID_Y + i_state.trunk_y);
        renderBackgroundAtPos(renderer, (SPRITE_WAVE_L_RIGHT + ((r_state.animationCycle/12 + 2) % 3)), WAVE_L_RIGHT_X + i_state.trunk_x, WAVE_L_RIGHT_Y + i_state.trunk_y);
        renderBackgroundAtPos(renderer, (SPRITE_WAVE_STONE + r_state.animationCycle/12), WAVE_STONE_X + i_state.trunk_x, WAVE_STONE_Y + i_state.trunk_y);
    }
    else
    {
        renderBackgroundAtPos(renderer, (SPRITE_WAVE_LEFT + r_state.animationCycle/12), WAVE_LEFT_X + i_state.trunk_x, WAVE_LEFT_Y + i_state.trunk_y);
        renderBackgroundAtPos(renderer, (SPRITE_WAVE_MID + ((r_state.animationCycle/12 + 1) % 3)), WAVE_MID_X + i_state.trunk_x, WAVE_MID_Y + i_state.trunk_y);
        renderBackgroundAtPos(renderer, (SPRITE_WAVE_RIGHT + ((r_state.animationCycle/12 + 2) % 3)), WAVE_RIGHT_X + i_state.trunk_x, WAVE_RIGHT_Y + i_state.trunk_y);
    }

    //weather is missing

    ++r_state.animationCycle;
    if (r_state.animationCycle >= 36)
       r_state.animationCycle = 0;
}

void SCRANTIC::ScranticParallelPlayer::renderBackgroundAtPos(SDL_Renderer *renderer, u_int16_t num, int32_t x, int32_t y, bool raft, bool holiday)
{
    SDL_Rect src, dest;
    SDL_Texture *bkg;
    if (!raft && !holiday)
        bkg = r_state.background->getImage(renderer, num, src);
    else if (raft)
        bkg = r_state.raft->getImage(renderer, num, src);
    else
        bkg = r_state.holiday->getImage(renderer, num, src);

    dest.x = x;
    dest.y = y;
    dest.w = src.w;
    dest.h = src.h;
    SDL_RenderCopy(renderer, bkg, &src, &dest);
}

void SCRANTIC::ScranticParallelPlayer::render()
{
    // first render background
    SDL_SetRenderTarget(r_state.renderer, r_state.bg);
    SDL_SetRenderDrawColor(r_state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(r_state.renderer);

    if (i_state.islandPos > 0)
    {
        if (i_state.night)
            SDL_RenderCopy(r_state.renderer, r_state.nightOcean, &r_state.oceanRect, &r_state.oceanRect);
        else
            SDL_RenderCopy(r_state.renderer, r_state.currentOcean, &r_state.oceanRect, &r_state.oceanRect);

        if (i_state.largerIsland)
        {
            renderBackgroundAtPos(r_state.renderer, SPRITE_L_ISLAND, L_ISLAND_X + i_state.trunk_x, L_ISLAND_Y + i_state.trunk_y);
            renderBackgroundAtPos(r_state.renderer, SPRITE_STONE, STONE_X + i_state.trunk_x, STONE_Y + i_state.trunk_y);
        }

        renderBackgroundAtPos(r_state.renderer, SPRITE_ISLAND, ISLAND_X + i_state.trunk_x, ISLAND_Y + i_state.trunk_y);
        renderBackgroundAtPos(r_state.renderer, SPRITE_TOP_SHADOW, TOP_SHADOW_X + i_state.trunk_x, TOP_SHADOW_Y + i_state.trunk_y);
        renderBackgroundAtPos(r_state.renderer, SPRITE_TRUNK, i_state.trunk_x, i_state.trunk_y);
        renderBackgroundAtPos(r_state.renderer, SPRITE_TOP, TOP_X + i_state.trunk_x, TOP_Y + i_state.trunk_y);
        renderBackgroundAtPos(r_state.renderer, 0, RAFT_X + i_state.trunk_x, RAFT_Y + i_state.trunk_y, true);

        animateBackground(r_state.renderer);
    }
    else
    {
        /*for (auto scene = r_state.currentScenes.begin(); scene != r_state.currentScenes.end(); ++scene)
            if ((*scene).currentScreen != NULL)
                SDL_RenderCopy(r_state.renderer, (*scene).currentScreen, &(*scene).screenRect, &(*scene).screenRect);*/
    }

    Uint32 c1, c2;

    // saved image
    /*for (auto scene = r_state.currentScenes.begin(); scene != r_state.currentScenes.end(); ++scene)
        SDL_RenderCopy(r_state.renderer, (*scene).savedImage, &r_state.fullRect, &r_state.fullRect);*/

    // background end

    // render each foreground
    /*for (auto scene = r_state.currentScenes.begin(); scene != r_state.currentScenes.end(); ++scene)
    {
        SDL_SetRenderTarget(r_state.renderer, (*scene).fg);
        SDL_SetRenderDrawColor(r_state.renderer, 0, 0, 0, 0);
        SDL_RenderClear(r_state.renderer);

        for (auto item = (*scene).renderItems.begin(); item != (*scene).renderItems.end(); ++item)
        {
            switch ((*item).itemType)
            {
            case RITEM_SPRITE:
                SDL_RenderCopyEx(r_state.renderer, (*item).tex, &(*item).src, &(*item).dest, 0, NULL, (SDL_RendererFlip)(*item).flags);
                break;
            case RITEM_LINE:
                SDL_SetRenderDrawColor(r_state.renderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(r_state.renderer, (*item).src.x, (*item).src.y, (*item).src.w, (*item).src.h);
                break;
            case RITEM_RECT:
                SDL_SetRenderDrawColor(r_state.renderer, r_state.palette[(*item).color.first].r, r_state.palette[(*item).color.first].g, r_state.palette[(*item).color.first].b, 255);
                SDL_RenderDrawRect(r_state.renderer, &(*item).src);
                SDL_SetRenderDrawColor(r_state.renderer, r_state.palette[(*item).color.second].r, r_state.palette[(*item).color.second].g, r_state.palette[(*item).color.second].b, 255);
                SDL_RenderFillRect(r_state.renderer, &(*item).src);
                break;
            case RITEM_ELLIPSE:
                c1 = r_state.palette[(*item).color.first].r * 0x10000
                        + r_state.palette[(*item).color.first].g * 0x100
                        + r_state.palette[(*item).color.first].b + 0xFF000000;
                c2 = r_state.palette[(*item).color.second].r * 0x10000
                        + r_state.palette[(*item).color.second].g * 0x100
                        + r_state.palette[(*item).color.second].b + 0xFF000000;
                filledEllipseColor(r_state.renderer, (*item).src.x, (*item).src.y, (*item).src.w, (*item).src.h, c2);
                ellipseColor(r_state.renderer, (*item).src.x, (*item).src.y, (*item).src.w, (*item).src.h, c1);
                break;
            default:
                std::cerr << "ERROR: Renderer: Unkown render item type!" << std::endl;
                break;
            }
        }

        SDL_SetRenderDrawColor(r_state.renderer, 0, 0, 0, 255);

        // save foreground rect
        if ((*scene).saveImage)
        {
            SDL_SetRenderTarget(r_state.renderer, (*scene).savedImage);
            if ((*scene).saveNewImage)
            {
                SDL_SetRenderDrawColor(r_state.renderer, 0, 0, 0, 0);
                SDL_RenderClear(r_state.renderer);
                (*scene).saveNewImage = false;
            }
            SDL_RenderCopy(r_state.renderer, (*scene).fg, &(*scene).saveRect, &(*scene).saveRect);
            (*scene).saveImage = false;
        }
    }*/

    // render everything to screen
    SDL_SetRenderTarget(r_state.renderer, r_state.rendererTarget);
    SDL_RenderClear(r_state.renderer);

    SDL_RenderCopy(r_state.renderer, r_state.bg, &r_state.fullRect, &r_state.fullRect);

    /*for (auto scene = r_state.currentScenes.begin(); scene != r_state.currentScenes.end(); ++scene)
    {
        if ((*scene).clipRegion.w > 0)
            SDL_RenderCopy(r_state.renderer, (*scene).fg, &(*scene).clipRegion, &(*scene).clipRegion);
        else
            SDL_RenderCopy(r_state.renderer, (*scene).fg, &r_state.fullRect, &r_state.fullRect);
    }*/

    if (r_state.displayMenu)
        renderMenu();
}

void SCRANTIC::ScranticParallelPlayer::resetPlayer()
{
    currentMovie = 0;

    /*for (auto scene = r_state.currentScenes.begin(); scene != r_state.currentScenes.end(); ++scene)
    {
        (*scene).queuedItems.clear();
        (*scene).renderItems.clear();
        SDL_DestroyTexture((*scene).savedImage);
        SDL_DestroyTexture((*scene).fg);
    }
    r_state.currentScenes.clear();*/

    //SDL_SetRenderTarget(r_state.renderer, r_state.savedImage);
    //SDL_SetRenderDrawColor(r_state.renderer, 0, 0, 0, 0);
    //SDL_RenderClear(r_state.renderer);
    SDL_SetRenderDrawColor(r_state.renderer, 0, 0, 0, 255);
    //r_state.saveNewImage = false;

    SDL_SetRenderTarget(r_state.renderer, r_state.rendererTarget);

    delay = 0;

    i_state.night = (bool)(std::rand() % 2);
    i_state.largerIsland = (bool)(std::rand() % 2);
    i_state.islandPos = NO_ISLAND;
    i_state.trunk_x = ISLAND_TEMP_X;
    i_state.trunk_y = ISLAND_TEMP_Y;

    r_state.animationCycle = 0;

    // random pick ocean (0-2)
    u_int8_t random = std::rand() % 3;
    std::string ocean = "OCEAN0"+std::to_string(random)+".SCR";
    r_state.currentOcean = static_cast<SCRFile *>(res->getResource(ocean))->getImage(r_state.renderer, r_state.oceanRect);

    //r_state.currentScreen = NULL;

    r_state.anyTTMSceneRunning = false;
    r_state.displayMenu = false;
    r_state.clear = false;

    res->resetTTMPositions();
}

/*void SCRANTIC::ScranticParallelPlayer::purgePlayer()
{
    //spriteList.clear();
    //ellipses.clear();
    //lines.clear();

    //r_state.savedBgRect.w = 0;
    //r_state.savedBgRect.h = 0;
    //r_state.savedFgRect.w = 0;
    //r_state.savedFgRect.h = 0;

    r_state.clipRegion.w = 0;
}*/

void SCRANTIC::ScranticParallelPlayer::LoadADS(std::string ads, int32_t num)
{
    if (ads == "")
        return;

    currentADS = static_cast<ADSFile *>(res->getResource(ads));
    if (currentADS == NULL)
        return;

    //purgePlayer();

    resetPlayer();

    r_state.currentMenuScreen = ads;

    if (num >= 0)
        StartADSMovie(num);
}

void SCRANTIC::ScranticParallelPlayer::StartADSMovie(u_int16_t num)
{
    if (currentADS == NULL)
        return;

    //purgePlayer();

    currentMovieFinished = false;
    currentMovie = num;
    r_state.currentMenuPos = num;
    lookForLabel = false;
    delay = 0;
    currentADS->resetScript();
}

u_int32_t SCRANTIC::ScranticParallelPlayer::getCurrentDelay()
{
    //for (auto scene = r_state.currentScenes.begin(); scene != r_state.currentScenes.end(); ++scene)
    //return r_state.currentScenes.front().delay;
    return 1000;
}

void SCRANTIC::ScranticParallelPlayer::AdvanceScript()
{
    Command cmd;
    TTMScene scene;
    RenderItem item;
    size_t randomPick;

    if (currentMovieFinished)
        return;

    if (!r_state.anyTTMSceneRunning)
    {
        do
        {
            if (lookForLabel)
            {
                cmd = currentADS->getNextCommand(currentMovie, labelRes, labelScene);
                lookForLabel = false;
            }
            else
                cmd = currentADS->getNextCommand(currentMovie);
            //std::cout << "ADS Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
            switch (cmd.opcode)
            {
            case CMD_ADD_INIT_TTM:
                scene.ttmName = currentADS->getResource(cmd.data.at(0));
                scene.ttm = static_cast<TTMFile *>(res->getResource(scene.ttmName));
                scene.resNo = cmd.data.at(0);
                scene.scene = cmd.data.at(1);
                scene.repeat = 1;
                scene.blob =  0;
                scene.wantsInit = true;
                scene.clipRegion.x = 0;
                scene.clipRegion.y = 0;
                scene.clipRegion.w = 0;
                scene.clipRegion.h = 0;
                scene.screenRect.x = 0;
                scene.screenRect.y = 0;
                scene.fg = SDL_CreateTexture(r_state.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_state.fullRect.w, r_state.fullRect.h);
                scene.savedImage = SDL_CreateTexture(r_state.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_state.fullRect.w, r_state.fullRect.h);
                SDL_SetTextureBlendMode(scene.fg, SDL_BLENDMODE_BLEND);
                SDL_SetTextureBlendMode(scene.savedImage, SDL_BLENDMODE_BLEND);

                //r_state.currentScenes.push_back(scene);
                break;
            case CMD_SKIP_IF_LAST:
                for (size_t i = 0; i < cmd.data.size(); i += 2)
                    if ((cmd.data.at(i) == labelRes) && (cmd.data.at(i+1) == labelScene))
                    {
                        currentADS->skip();
                        break;
                    }
                break;
            case CMD_PLAY_MOVIE:
                /*if (r_state.currentScenes.size())
                {
                    r_state.anyTTMSceneRunning = true;
                    AdvanceScript();
                }*/
                return;
            case CMD_ADD_TTM: //TTM Scene ??? Repeat
                scene.ttmName = currentADS->getResource(cmd.data.at(0));
                scene.ttm = static_cast<TTMFile *>(res->getResource(scene.ttmName));
                scene.resNo = cmd.data.at(0);
                scene.scene = cmd.data.at(1);
                scene.repeat = cmd.data.at(3);
                scene.blob =  cmd.data.at(2);
                scene.wantsInit = false;
                scene.clipRegion.x = 0;
                scene.clipRegion.y = 0;
                scene.clipRegion.w = 0;
                scene.clipRegion.h = 0;
                scene.screenRect.x = 0;
                scene.screenRect.y = 0;
                scene.fg = SDL_CreateTexture(r_state.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_state.fullRect.w, r_state.fullRect.h);
                scene.savedImage = SDL_CreateTexture(r_state.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, r_state.fullRect.w, r_state.fullRect.h);
                SDL_SetTextureBlendMode(scene.fg, SDL_BLENDMODE_BLEND);
                SDL_SetTextureBlendMode(scene.savedImage, SDL_BLENDMODE_BLEND);

                if (scene.blob != 0)
                    std::cout << currentADS->filename << ": TTM Movie with blob " << (int32_t)scene.blob << std::endl;
                /*if (isRandom)
                    //pickRandom.push_back(scene);
                else
                    //r_state.currentScenes.push_back(scene);*/
                break;
            case CMD_RANDOM_START:
                isRandom = true;
                //pickRandom.clear();
                break;
            case CMD_RANDOM_END:
                isRandom = false;
                //randomPick = std::rand() % pickRandom.size();
                //scene = pickRandom[randomPick];
                std::cout << "Random pick: " << randomPick << std::endl;
                //pickRandom.clear();
                --(scene.repeat);
                //r_state.currentScenes.push_back(scene);
                break;
            case CMD_PLAY_ADS_MOVIE:
                if (lastMovie == cmd.data.at(0))
                    break;
                else
                {
                    queuedMovie = currentMovie;
                    currentMovie = cmd.data.at(0);
                }
                break;
            case CMD_SET_SCENE:
                std::cout << "Play ADS Movie: " << cmd.name << std::endl;
                break;
            case CMD_INTER_NOTFOUND:
                std::cerr << currentADS->filename << ": Error tried to load unkown ADS Movie" << std::endl;
                return;
            case CMD_INTER_END:
                std::cout << currentADS->filename << ": Finished ADS Movie: " << currentMovie << std::endl;
                if (queuedMovie > -1)
                {
                    lastMovie = currentMovie;
                    currentMovie = queuedMovie;
                    queuedMovie = -1;
                }
                else
                {
                    currentMovieFinished = true;
                    lastMovie = currentMovie;
                }
                return;
            default:
                std::cout << "ADS Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
                break;
            }
        } while (1);
    }
    else
    {
        bool switchTTM = false;
        /*for (auto scene = r_state.currentScenes.begin(); scene != r_state.currentScenes.end(); ++scene)
        {
            switchTTM = false;
            do
            {
                cmd = (*scene).ttm->getNextCommand((*scene).scene, (*scene).newScene);
                (*scene).newScene = false;
                //std::cout << "TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
                switch (cmd.opcode)
                {
                case CMD_PURGE:
                    //purgePlayer();
                    (*scene).clipRegion.w = 0;
                    break;
                case CMD_UPDATE:
                    (*scene).renderItems.splice((*scene).renderItems.end(), (*scene).queuedItems);
                    switchTTM = true;
                    break;
                case CMD_DELAY:
                    (*scene).delay = cmd.data.at(0) * 20;
                    break;
                case CMD_SEL_SLOT_IMG:
                    (*scene).currentImgSlot = cmd.data.at(0);
                    break;
                case CMD_SEL_SLOT_PAL:
                    (*scene).currentPalSlot = cmd.data.at(0);
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
                    break;* /
                case CMD_CLIP_REGION:
                    (*scene).clipRegion.x = (int16_t)cmd.data.at(0);
                    (*scene).clipRegion.y = (int16_t)cmd.data.at(1);
                    (*scene).clipRegion.w = cmd.data.at(2) - (*scene).clipRegion.x;
                    (*scene).clipRegion.h = cmd.data.at(3) - (*scene).clipRegion.y;
                    if ((*scene).clipRegion.x + (*scene).clipRegion.w >= 640)
                        (*scene).clipRegion.w = 640 - (*scene).clipRegion.x;
                    if ((*scene).clipRegion.y + (*scene).clipRegion.h >= 480)
                        (*scene).clipRegion.h = 480 - (*scene).clipRegion.y;
                    break;
                case CMD_SAVE_IMAGE:
                case CMD_SAVE_IMAGE_NEW:
                    (*scene).saveRect.x = (int16_t)cmd.data.at(0);
                    (*scene).saveRect.y = (int16_t)cmd.data.at(1);
                    (*scene).saveRect.w = cmd.data.at(2);
                    (*scene).saveRect.h = cmd.data.at(3);
                    (*scene).saveImage = true;
                    if (cmd.opcode == CMD_SAVE_IMAGE_NEW)
                        (*scene).saveNewImage = true;
                    (*scene).renderItems.splice((*scene).renderItems.end(), (*scene).queuedItems);
                    render();
                    break;
                case CMD_DRAW_PIXEL:
                    item.src.x = (int16_t)cmd.data.at(0);
                    item.src.y = (int16_t)cmd.data.at(1);
                    item.src.w = 2;
                    item.src.h = 2;
                    item.color = (*scene).currentColor;
                    item.itemType = RITEM_RECT;
                    (*scene).queuedItems.push_back(item);
                    break;
                case CMD_DRAW_LINE:
                    item.src.x = (int16_t)cmd.data.at(0);
                    item.src.y = (int16_t)cmd.data.at(1);
                    item.src.w = (int16_t)cmd.data.at(2);
                    item.src.h = (int16_t)cmd.data.at(3);
                    item.color = (*scene).currentColor;
                    item.itemType = RITEM_LINE;
                    (*scene).queuedItems.push_back(item);
                    break;
                case CMD_SET_COLOR:
                    (*scene).currentColor = std::make_pair(cmd.data.at(0), cmd.data.at(1));
                    break;
                case CMD_DRAW_RECTANGLE:
                    item.src.x = (int16_t)cmd.data.at(0);
                    item.src.y = (int16_t)cmd.data.at(1);
                    item.src.w = cmd.data.at(2);
                    item.src.h = cmd.data.at(3);
                    item.color = (*scene).currentColor;
                    item.itemType = RITEM_RECT;
                    (*scene).queuedItems.push_back(item);
                    std::cout << "TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
                    break;
                case CMD_DRAW_ELLIPSE:
                    item.src.w = cmd.data.at(2)/2;
                    item.src.h = cmd.data.at(3)/2;
                    item.src.x = (int16_t)cmd.data.at(0) + item.src.w;
                    item.src.y = (int16_t)cmd.data.at(1) + item.src.h;
                    item.color = (*scene).currentColor;
                    item.itemType = RITEM_ELLIPSE;
                    (*scene).queuedItems.push_back(item);
                    break;
                case CMD_DRAW_SPRITE:
                case CMD_DRAW_SPRITE_MIRROR:
                    if ((*scene).images[cmd.data.at(3)] != NULL)
                    {
                        item.dest.x = (int16_t)cmd.data.at(0);
                        item.dest.y = (int16_t)cmd.data.at(1);
                        item.num = cmd.data.at(2);
                        item.tex = (*scene).images[cmd.data.at(3)]->getImage(r_state.renderer, item.num, item.src);
                        if (item.tex == NULL)
                        {
                            std::cerr << (*scene).ttm->filename << ": Error tried to access non existing sprite number!" << std::endl;
                            std::cout << ">>> TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
                            break;
                        }
                        item.dest.w = item.src.w;
                        item.dest.h = item.src.h;
                        item.flags = 0;
                        if (cmd.opcode == CMD_DRAW_SPRITE_MIRROR)
                            item.flags |= SPRITE_MIRROR;
                        item.itemType = RITEM_SPRITE;
                        (*scene).queuedItems.push_back(item);
                    }
                    else
                    {
                        std::cerr << (*scene).ttm->filename << ": Error tried to access unloaded image slot!" << std::endl;
                        std::cout << ">>> TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
                    }
                    break;
                case CMD_CLEAR_RENDERER:
                    (*scene).renderItems.clear();
                    break;
                case CMD_PLAY_SOUND:
                    if ((cmd.data.at(0) < 1) || (cmd.data.at(0) > MAX_AUDIO_SAMPLES))
                        break;
                    Mix_HaltChannel(-1);
                    Mix_PlayChannel(-1, audioSamples[cmd.data.at(0)-1], 0);
                    break;
                case CMD_LOAD_SCREEN:
                    (*scene).currentScreen = static_cast<SCRFile *>(res->getResource(cmd.name))->getImage(r_state.renderer, (*scene).screenRect);
                    if (cmd.name == "ISLETEMP.SCR")
                    {
                        i_state.islandPos = ISLAND_RIGHT;
                        i_state.trunk_x = ISLAND_TEMP_X;
                        i_state.trunk_y = ISLAND_TEMP_Y;
                    }
                    else if (cmd.name == "ISLAND2.SCR")
                    {
                        i_state.islandPos = ISLAND_LEFT;
                        i_state.trunk_x = ISLAND2_X;
                        i_state.trunk_y = ISLAND2_Y;
                    }
                    else
                        i_state.islandPos = NO_ISLAND;
                    (*scene).screenRect.x = 0;
                    (*scene).screenRect.y = 0;

                    (*scene).renderItems.clear();
                    break;
                case CMD_LOAD_BITMAP:
                    (*scene).images[(*scene).currentImgSlot] = static_cast<BMPFile *>(res->getResource(cmd.name));
                    break;
                case CMD_INTER_NOTFOUND:
                    std::cerr << (*scene).ttm->filename << ": Error tried to load unkown TTM Scene" << std::endl;
                    return;
                case CMD_INTER_END:
                    std::cout << (*scene).ttm->filename << ": Finished TTM Scene: " << (*scene).scene << std::endl;

                    lookForLabel = true;
                    labelRes = (*scene).resNo;
                    labelScene = (*scene).scene;

                    if ((*scene).repeat)
                    {
                        --((*scene).repeat);
                        (*scene).newScene = true;
                    }
                    else
                    {
                        SDL_DestroyTexture((*scene).savedImage);
                        SDL_DestroyTexture((*scene).fg);
                        r_state.currentScenes.erase(scene);
                    }

                    if (!r_state.currentScenes.size())
                        r_state.anyTTMSceneRunning = false;

                    AdvanceScript();
                    switchTTM = true;
                    break;
                case CMD_LOAD_PALETTE:
                    // ignored - there is only palette...
                    break;
                default:
                    std::cout << "TTM Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;
                    break;
                }
            } while (!switchTTM);
        }*/
    }
}



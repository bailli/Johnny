#include "Robinson.h"

#include <ctime>
#include <algorithm>

#include "defines.h"

#ifdef WIN32
#include <SDL2_gfxPrimitives.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif

SCRANTIC::Robinson::Robinson(std::string ResMap, std::string ScrExe)
    : res(NULL), audioPlayer(NULL), renderMenu(false), renderer(NULL),
      movieRunning(false), animationCycle(0), islandPos(NO_ISLAND), ads(NULL),
      queuedMovie(0), currentMovie(0), delay(0), delayTicks(0)
{
    std::cout << "--------------- Hello from Robinson Crusoe!---------------" << std::endl;

    res = new RESFile(ResMap);
    audioPlayer = new RIFFPlayer(ScrExe);

    std::srand(std::time(0));

    PALFile *pal = static_cast<PALFile *>(res->getResource("JOHNCAST.PAL"));
    for (auto it = res->resourceMap.begin(); it != res->resourceMap.end(); ++it)
        if (it->second.filetype == "BMP")
            static_cast<BMPFile *>(it->second.handle)->setPalette(pal->getPalette(), 256);
        else if (it->second.filetype == "SCR")
            static_cast<SCRFile *>(it->second.handle)->setPalette(pal->getPalette(), 256);
    palette = pal->getPalette();

#ifdef DUMP_ADS
    std::string adsstring;
    std::string num;
    ADSFile *dump;
    for (size_t i = 0; i < res->ADSFiles.size(); ++i)
    {
        adsstring = res->ADSFiles.at(i);
        dump = static_cast<ADSFile *>(res->getResource(adsstring));

        std::cout << "Filename: " << dump->filename << std::endl;

        for (auto it = dump->tagList.begin(); it != dump->tagList.end(); ++it)
        {
            num = SCRANTIC::BaseFile::hex_to_string(it->first, std::dec);
            for (int j = num.size(); j < 3; ++j)
                num = " " + num;
            std::cout << "TAG ID " << num << ": " << it->second << std::endl;
        }
        std::cout << std::endl;

        std::string cmdString;
        std::string ttmName;
        Command *cmd;
        TTMFile *ttm;
        for (auto it = dump->script.begin(); it != dump->script.end(); ++it)
        {
            std::cout << "Movie number: " << it->first << " - 0x" << SCRANTIC::BaseFile::hex_to_string(it->first, std::hex) << std::endl;
            for (size_t pos = 0; pos < it->second.size(); ++pos)
            {
                num = SCRANTIC::BaseFile::hex_to_string(pos, std::dec);
                for (int j = num.size(); j < 3; ++j)
                    num = " " + num;

                cmdString = SCRANTIC::BaseFile::commandToString(it->second[pos], true);
                cmd = &(it->second[pos]);
                switch (cmd->opcode)
                {
                case CMD_ADD_INIT_TTM:
                case CMD_ADD_TTM:
                case CMD_KILL_TTM:
                case CMD_UNK_1370:
                    ttmName = dump->getResource(cmd->data.at(0));
                    ttm = static_cast<TTMFile *>(res->getResource(ttmName));
                    //cmdString += "| " + ttmName + " - " + ttm->getTag(cmd->data.at(1));
                    cmdString += "| " + ttm->getTag(cmd->data.at(1));
                    break;
                case CMD_TTM_LABEL:
                case CMD_SKIP_IF_LAST:
                    for (size_t j = 0; j < cmd->data.size(); j+=2)
                    {
                        ttmName = dump->getResource(cmd->data.at(j));
                        ttm = static_cast<TTMFile *>(res->getResource(ttmName));
                        cmdString += "| " + ttm->getTag(cmd->data.at(j+1)) + " ";
                    }
                    break;

                }

                std::cout << num << ": " << cmdString << std::endl;

            }

            std::cout << std::endl;
        }

        std::cout << std::endl;
        std::cout << std::endl;

    }
#endif

}

SCRANTIC::Robinson::~Robinson()
{
    delete palette;

    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(fgTexture);
    SDL_DestroyTexture(saveTexture);

    if (audioPlayer != NULL)
        delete audioPlayer;

    if (res != NULL)
        delete res;

    std::cout << "-------------- Goodbye from Robinson Crusoe!--------------" << std::endl;
}

void SCRANTIC::Robinson::displaySplash()
{
    //display splash
    SDL_Rect splashRect;
    splashRect.x = 0;
    splashRect.y = 0;
    SDL_Texture *splash = static_cast<SCRFile *>(res->getResource("INTRO.SCR"))->getImage(renderer, splashRect);
    SDL_RenderCopy(renderer, splash, &splashRect, &splashRect);
    SDL_RenderPresent(renderer);
}

void SCRANTIC::Robinson::initRenderer(SDL_Renderer *rendererSDL)
{
    //renderer and target
    renderer = rendererSDL;
    rendererTarget = SDL_GetRenderTarget(renderer);

    displaySplash();

    // init island
    islandNight = std::rand() % 2;
    islandLarge = std::rand() % 2;
    islandPos = NO_ISLAND;
    islandTrunk.x = ISLAND_TEMP_X;
    islandTrunk.y = ISLAND_TEMP_Y;

    // random pick ocean (0-2)
    oceanRect.x = 0;
    oceanRect.y = 0;
    u_int8_t random = std::rand() % 3;
    std::string ocean = "OCEAN0"+std::to_string(random)+".SCR";

    // load SCR files
    oceanTexture = static_cast<SCRFile *>(res->getResource(ocean))->getImage(renderer, oceanRect);
    oceanNightTexture = static_cast<SCRFile *>(res->getResource("NIGHT.SCR"))->getImage(renderer, oceanRect);

    // load BMP files
    backgroundBMP = static_cast<BMPFile *>(res->getResource("BACKGRND.BMP"));
    raftBMP = static_cast<BMPFile *>(res->getResource("MRAFT.BMP"));
    holidayBMP = static_cast<BMPFile *>(res->getResource("HOLIDAY.BMP"));

    scrTexture = NULL;

    // many rects
    fullRect.x = 0;
    fullRect.y = 0;
    fullRect.w = 640;
    fullRect.h = 480;

    // width/heigth gets filled in on load
    screenRect.x = 0;
    screenRect.y = 0;

    // create background and foreground texture
    // better pixel format?
    bgTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, fullRect.w, fullRect.h);
    fgTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, fullRect.w, fullRect.h);
    saveTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, fullRect.w, fullRect.h);
    SDL_SetTextureBlendMode(fgTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(saveTexture, SDL_BLENDMODE_BLEND);
}

void SCRANTIC::Robinson::initMenu(TTF_Font *font)
{
    //generate menu textures
    ADSFile *ads;

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

        menuPos.insert(std::make_pair(adsstring, std::map<u_int16_t, SDL_Rect>()));


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
                menuPos.at(adsstring).insert(std::make_pair(id, rect));
                rect.x += 3;
                rect.y += 3;
                rect.h -= 6;
            }

        }

        tex = SDL_CreateTextureFromSurface(renderer, menu);

        if (tex == NULL)
            std::cerr << "ERROR: Renderer: Could not convert menu surface to to texture: " << res->ADSFiles.at(i) << std::endl;
        else
        {
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureAlphaMod(tex, 200);
        }

        SDL_FreeSurface(menu);

        menuScreen.insert(std::make_pair(adsstring, tex));
    }
}

bool SCRANTIC::Robinson::navigateMenu(SDL_Keycode key)
{
    size_t i;

    switch (key)
    {
    case SDLK_LEFT:
    case SDLK_RIGHT:
        for (i = 0; i < res->ADSFiles.size(); ++i)
            if (res->ADSFiles.at(i) == currentMenuScreen)
                break;

        if (i >= res->ADSFiles.size())
        {
            std::cerr << "Menu screen not found! " << currentMenuScreen << std::endl;
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

        currentMenuScreen = res->ADSFiles.at(i);
        currentMenuPos = menuPos[currentMenuScreen].begin()->first;
        break;

    case SDLK_RETURN:
        loadMovie(currentMenuScreen, currentMenuPos);
        renderMenu = false;
        movieRunning = true;
        return true;

    case SDLK_UP:
    case SDLK_DOWN:
        auto it = menuPos[currentMenuScreen].find(currentMenuPos);
        if (it == menuPos[currentMenuScreen].end())
        {
            std::cerr << "Menu position not found! " << currentMenuScreen << std::endl;
            return false;
        }

        if (key == SDLK_UP)
        {
            if (it == menuPos[currentMenuScreen].begin())
                it = menuPos[currentMenuScreen].end();
            --it;
        }
        else
        {
            ++it;
            if (it == menuPos[currentMenuScreen].end())
                it = menuPos[currentMenuScreen].begin();
        }
        currentMenuPos = it->first;
        break;
    }

    return false;
}

void SCRANTIC::Robinson::menuRenderer()
{
    SDL_Texture *tex;

    auto it = menuScreen.find(currentMenuScreen);
    if (it == menuScreen.end())
    {
        std::cerr << "Menu screen not found! " << currentMenuScreen << std::endl;
        return;
    }

    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
    SDL_RenderFillRect(renderer, &menuPos[currentMenuScreen][currentMenuPos]);
    SDL_RenderCopy(renderer, it->second, NULL, NULL);
}


bool SCRANTIC::Robinson::loadMovie(std::string adsName, u_int16_t num)
{
    ads = static_cast<ADSFile *>(res->getResource(adsName));

    if (!ads)
        return false;

    script = ads->getFullMovie(num);
    if (script.size())
        scriptPos = 0;
    else
        return false;

    name = ads->filename;
    currentMenuScreen = adsName;
    currentMenuPos = num;
    currentMovie = num;

    labels = ads->getMovieLabels(num);

    resetPlayer();

    return true;
}

void SCRANTIC::Robinson::startMovie()
{
    movieRunning = true;
    renderMenu = false;
    delay = 0;
}

void SCRANTIC::Robinson::resetPlayer()
{
    //currentMovie = 0;

    for (auto it = ttmScenes.begin(); it != ttmScenes.end(); ++it)
        delete (*it);

    ttmScenes.clear();

    islandNight = std::rand() % 2;
    u_int8_t random = std::rand() % 3;
    std::string ocean = "OCEAN0"+std::to_string(random)+".SCR";
    oceanTexture = static_cast<SCRFile *>(res->getResource(ocean))->getImage(renderer, oceanRect);
    islandLarge = std::rand() % 2;

    movieRunning = false;
    scrTexture = NULL;
    islandPos= NO_ISLAND;

    lastTTMs.clear();

    delay = 0;
    delayTicks = 0;

    for (int i = 0; i < MAX_IMAGES; ++i)
        images[i] = NULL;

    SDL_SetRenderTarget(renderer, saveTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, rendererTarget);
}

void SCRANTIC::Robinson::renderBackgroundAtPos(u_int16_t num, int32_t x, int32_t y, bool raft, bool holiday)
{
    SDL_Rect src, dest;
    SDL_Texture *bkg;
    if (!raft && !holiday)
        bkg = backgroundBMP->getImage(renderer, num, src);
    else if (raft)
        bkg = raftBMP->getImage(renderer, num, src);
    else
        bkg = holidayBMP->getImage(renderer, num, src);

    dest.x = x;
    dest.y = y;
    dest.w = src.w;
    dest.h = src.h;
    SDL_RenderCopy(renderer, bkg, &src, &dest);
}

void SCRANTIC::Robinson::animateBackground()
{
    if (!islandPos)
        return;

    if (islandLarge)
    {
        renderBackgroundAtPos((SPRITE_WAVE_L_LEFT + animationCycle/12), WAVE_L_LEFT_X + islandTrunk.x, WAVE_L_LEFT_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_L_MID + ((animationCycle/12 + 1) % 3)), WAVE_L_MID_X + islandTrunk.x, WAVE_L_MID_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_L_RIGHT + ((animationCycle/12 + 2) % 3)), WAVE_L_RIGHT_X + islandTrunk.x, WAVE_L_RIGHT_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_STONE + animationCycle/12), WAVE_STONE_X + islandTrunk.x, WAVE_STONE_Y + islandTrunk.y);
    }
    else
    {
        renderBackgroundAtPos((SPRITE_WAVE_LEFT + animationCycle/12), WAVE_LEFT_X + islandTrunk.x, WAVE_LEFT_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_MID + ((animationCycle/12 + 1) % 3)), WAVE_MID_X + islandTrunk.x, WAVE_MID_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_RIGHT + ((animationCycle/12 + 2) % 3)), WAVE_RIGHT_X + islandTrunk.x, WAVE_RIGHT_Y + islandTrunk.y);
    }

    //weather is missing

    ++animationCycle;
    if (animationCycle >= 36)
       animationCycle = 0;
}

void SCRANTIC::Robinson::render()
{
    SDL_Rect tmpRect;
    u_int8_t save;

    // first render background
    SDL_SetRenderTarget(renderer, bgTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (islandPos > 0)
    {
        if (islandNight)
            SDL_RenderCopy(renderer, oceanNightTexture, &oceanRect, &oceanRect);
        else
            SDL_RenderCopy(renderer, oceanTexture, &oceanRect, &oceanRect);

        if (islandLarge)
        {
            renderBackgroundAtPos(SPRITE_L_ISLAND, L_ISLAND_X + islandTrunk.x, L_ISLAND_Y + islandTrunk.y);
            renderBackgroundAtPos(SPRITE_STONE, STONE_X + islandTrunk.x, STONE_Y + islandTrunk.y);
        }

        renderBackgroundAtPos(SPRITE_ISLAND, ISLAND_X + islandTrunk.x, ISLAND_Y + islandTrunk.y);
        renderBackgroundAtPos(SPRITE_TOP_SHADOW, TOP_SHADOW_X + islandTrunk.x, TOP_SHADOW_Y + islandTrunk.y);
        renderBackgroundAtPos(SPRITE_TRUNK, islandTrunk.x, islandTrunk.y);
        renderBackgroundAtPos(SPRITE_TOP, TOP_X + islandTrunk.x, TOP_Y + islandTrunk.y);
        renderBackgroundAtPos(0, RAFT_X + islandTrunk.x, RAFT_Y + islandTrunk.y, true);

        animateBackground();
    }
    else
    {
        if (scrTexture != NULL)
            SDL_RenderCopy(renderer, scrTexture, &screenRect, &screenRect);
    }

    // saved image
    SDL_SetRenderTarget(renderer, saveTexture);
    for (auto it = ttmScenes.begin(); it != ttmScenes.end(); ++it)
    {
        save = (*it)->needsSave();
        if (save)
        {
            if (save == 2)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);
            }
            SDL_RenderCopy(renderer, (*it)->savedImage, &fullRect, &fullRect);
        }

        //pre render foreground
        (*it)->renderForeground();
    }
    // background end

    SDL_Rect tmp;
    tmp.w = 320;
    tmp.h = 240;
    tmp.x = 0;
    tmp.y = 0;
    // render everything to screen
    SDL_SetRenderTarget(renderer, rendererTarget);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, bgTexture, &fullRect, &fullRect);
    SDL_RenderCopy(renderer, saveTexture, &fullRect, &fullRect);

    std::string scrName;
    int16_t audio;

    for (auto it = ttmScenes.begin(); it != ttmScenes.end(); ++it)
    {
        if ((*it)->isClipped())
        {
            tmpRect = (*it)->getClipRect();
            SDL_RenderCopy(renderer, (*it)->fg, &tmpRect, &tmpRect);
        }
        else
            SDL_RenderCopy(renderer, (*it)->fg, &fullRect, &fullRect);

        audio = (*it)->getSample();
        if (audio != -1)
            audioPlayer->play(audio);

        scrName = (*it)->getSCRName();
        if (scrName != "")
        {
            //std::string scrName = (*it)->getSCRName();
            scrTexture = static_cast<SCRFile *>(res->getResource(scrName))->getImage(renderer, screenRect);
            if (scrName == "ISLETEMP.SCR")
            {
                islandPos = ISLAND_RIGHT;
                islandTrunk.x = ISLAND_TEMP_X;
                islandTrunk.y = ISLAND_TEMP_Y;
            }
            else if (scrName == "ISLAND2.SCR")
            {
                islandPos = ISLAND_LEFT;
                islandTrunk.x = ISLAND2_X;
                islandTrunk.y = ISLAND2_Y;
            }
            else
                islandPos = NO_ISLAND;

            screenRect.x = 0;
            screenRect.y = 0;
        }
    }

    if (renderMenu)
        menuRenderer();
}

void SCRANTIC::Robinson::addTTM(Command cmd)
{
    u_int16_t sceneNum;
    int16_t repeat;
    if (cmd.opcode == CMD_ADD_INIT_TTM)
    {
        repeat = 0;
        sceneNum = 0;
    }
    else
    {
        sceneNum = cmd.data.at(1);
        repeat = cmd.data.at(2);
        if (repeat)
            --repeat;
        if (cmd.data.at(2) != 0)
            std::cout << name << ": TTM Movie with blob " << (int16_t)cmd.data.at(2) << std::endl;
    }

    //check if a TTM matching this hash already exists - if it does do nothing
    std::pair<u_int16_t, u_int16_t> hash = std::make_pair(cmd.data.at(0), cmd.data.at(1));
    std::list<TTMPlayer *>::iterator it = ttmScenes.begin();
    while (it != ttmScenes.end())
    {
        if ((*it)->getHash() == hash)
            return;
        else
            ++it;
    }

    TTMPlayer *ttm = new TTMPlayer(ads->getResource(cmd.data.at(0)), cmd.data.at(0), sceneNum, repeat, res, images, palette, renderer);

    // this assumes the only relavant action in the init scripts
    // is to load a SCR! all other actions are lost
    if (cmd.opcode == CMD_ADD_INIT_TTM)
    {
        std::string scrName;

        do
        {
            ttm->advanceScript();
            scrName = ttm->getSCRName();
            if (scrName != "")
            {
                scrTexture = static_cast<SCRFile *>(res->getResource(scrName))->getImage(renderer, screenRect);
                if (scrName == "ISLETEMP.SCR")
                {
                    islandPos = ISLAND_RIGHT;
                    islandTrunk.x = ISLAND_TEMP_X;
                    islandTrunk.y = ISLAND_TEMP_Y;
                }
                else if (scrName == "ISLAND2.SCR")
                {
                    islandPos = ISLAND_LEFT;
                    islandTrunk.x = ISLAND2_X;
                    islandTrunk.y = ISLAND2_Y;
                }
                else
                    islandPos = NO_ISLAND;

                screenRect.x = 0;
                screenRect.y = 0;
            }
        }
        while (!ttm->isFinished());

        delete ttm;
        return;
    }

    ttmScenes.push_back(ttm);
}

bool SCRANTIC::Robinson::setPosToLabel(std::pair<u_int16_t, u_int16_t> lastPlayed)
{
    auto it = labels.find(lastPlayed);
    if (it == labels.end())
        return false;

    scriptPos = it->second;
    return true;
}

void SCRANTIC::Robinson::runTTMs()
{
    TTMPlayer *ttm;
    u_int16_t newDelay = 100;
    u_int16_t oldDelay = delay;
    u_int32_t ticks;

    if (!oldDelay)
        oldDelay = 100;

    auto it = ttmScenes.begin();
    while (it != ttmScenes.end())
    {
        ttm = (*it);
        ticks = ttm->getRemainigDelay(oldDelay);

        if (!ticks)
        {
            ttm->advanceScript();

            newDelay = ttm->getDelay();

            if (newDelay && (newDelay < delay))
                delay = newDelay;
        }

        if (ttm->isFinished())
        {
            lastTTMs.push_back(ttm->getHash());
            delete ttm;
            it = ttmScenes.erase(it);
            continue;
        }
        else
            ++it;


    }
}

void SCRANTIC::Robinson::advanceScripts()
{    
    if (ttmScenes.size())
    {
        runTTMs();
        if (lastTTMs.size())
        {
            for (auto it = lastTTMs.begin(); it != lastTTMs.end(); ++it)
                advanceADSScript((*it));
            lastTTMs.clear();
        }
    }
    else
        advanceADSScript();
}

void SCRANTIC::Robinson::advanceADSScript(std::pair<u_int16_t, u_int16_t> lastPlayed)
{
    if (!script.size())
        return;

    if (!movieRunning)
        return;

    if (scriptPos >= script.size())
    {
        movieRunning = false;
        return;
    }

    Command cmd;
    std::vector<Command> pickRandom;

    bool firstRun = false;
    bool isRandom = false;
    size_t randomPick;

    std::pair<u_int16_t, u_int16_t> hash;
    std::list<TTMPlayer *>::iterator it;

    //on first run go till PLAY_MOVIE
    //next runs check if label for last finished TTM exists and go there
    //otherwise do nothing
    //also: run ADD_TTM_INIT immediately

    if (scriptPos == 0)
        firstRun = true;
    else
    {
        if (!setPosToLabel(lastPlayed))
        {
            //std::cout << name << ": no label found for: " << lastPlayed.first << " " << lastPlayed.second << std::endl;
            if (!ttmScenes.size())
                movieRunning = false;
            return;
        }
    }

    for (; scriptPos < script.size(); ++scriptPos)
    {
        cmd = script[scriptPos];

        //std::cout << "ADS Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;

        switch (cmd.opcode)
        {
        case CMD_ADD_INIT_TTM:
            addTTM(cmd);
            break;

        case CMD_SKIP_IF_LAST:
            for (size_t i = 0; i < cmd.data.size(); i += 2)
                if ((cmd.data.at(i) == lastPlayed.first) && (cmd.data.at(i+1) == lastPlayed.second))
                {
                    //correct ?
                    while (script[scriptPos++].opcode != CMD_PLAY_MOVIE) { ; }
                    break;
                }
            break;

        case CMD_PLAY_MOVIE:
            runTTMs();
            return;

        case CMD_ADD_TTM: //TTM Scene ??? Repeat
            if (isRandom)
                pickRandom.push_back(cmd);
            else
                addTTM(cmd);
            break;

        case CMD_KILL_TTM:
            hash = std::make_pair(cmd.data.at(0), cmd.data.at(1));
            it = ttmScenes.begin();
            while (it != ttmScenes.end())
            {
                if ((*it)->getHash() == hash)
                {
                    (*it)->kill();
                    std::cout << "ADS Command: Kill movie " << hash.first << " " << hash.second << std::endl;
                    break;
                }
                else
                    ++it;
            }
            break;

        case CMD_RANDOM_START:
            isRandom = true;
            pickRandom.clear();
            break;

        case CMD_RANDOM_END:
            isRandom = false;
            randomPick = std::rand() % pickRandom.size();
            std::cout << "Random pick: " << randomPick << std::endl;
            addTTM(pickRandom[randomPick]);
            break;

        case CMD_PLAY_ADS_MOVIE:
            if (currentMovie == cmd.data.at(0))
                break;
            else
            {
                queuedMovie = currentMovie;
                queuedPos = ++scriptPos;
                loadMovie(name, cmd.data.at(0));
            }
            break;

        case CMD_SET_SCENE:
            std::cout << "Play ADS Movie: " << cmd.name << std::endl;
            break;

        default:
            std::cout << "ADS Command: " << SCRANTIC::BaseFile::commandToString(cmd, true) << std::endl;
            break;
        }
    }

    std::cout << name << ": Finished ADS Movie: " << currentMovie << std::endl;

    if (queuedMovie)
    {
        loadMovie(name, queuedMovie);
        queuedMovie = 0;
        scriptPos = queuedPos;
        advanceADSScript(lastPlayed);
        return;
    }

    movieRunning = false;

    return;
}

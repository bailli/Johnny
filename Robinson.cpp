#include "Robinson.h"

#include <ctime>
#include <algorithm>

#include "defines.h"

#ifdef WIN32
#include <SDL2_gfxPrimitives.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#endif

SCRANTIC::Robinson::Robinson(const std::string &path, bool readUnpacked)
    : res(NULL),
      audioPlayer(NULL),
      menu(NULL),
      renderer(NULL),
      ads(NULL),
      animationCycle(0),
      queuedMovie(0),
      currentMovie(0),
      islandPos(NO_ISLAND),
      movieRunning(false),
      delay(0),
      delayTicks(0),
      renderMenu(false),
      scriptPos(0),
      rendererTarget(NULL),
      islandNight(false),
      islandLarge(false),
      bgTexture(NULL),
      fgTexture(NULL),
      saveTexture(NULL),
      oceanTexture(NULL),
      oceanNightTexture(NULL),
      scrTexture(NULL),
      backgroundBMP(NULL),
      holidayBMP(NULL),
      raftBMP(NULL),
      queuedPos(0) {

    std::cout << "--------------- Hello from Robinson Crusoe!---------------" << std::endl;

    res = new RESFile(path + "RESOURCE.MAP", readUnpacked);
    audioPlayer = new RIFFPlayer(path, readUnpacked);

    std::srand(std::time(0));

    palette = res->setPaletteForAllGraphicResources("JOHNCAST.PAL");

#ifdef DUMP_ADS
    std::string adsstring;
    std::string num;
    std::map<std::string, std::vector<std::string>> items = res->getMovieList();
    ADSFile *dump;
    for (auto it = items.begin(); it != items.end(); ++it) {
        adsstring = it->first;
        dump = static_cast<ADSFile *>(res->getResource(adsstring));

        std::cout << "Filename: " << dump->filename << std::endl;

        for (auto it = dump->tagList.begin(); it != dump->tagList.end(); ++it) {
            num = SCRANTIC::BaseFile::hexToString(it->first, std::dec);
            for (size_t j = num.size(); j < 3; ++j) {
                num = " " + num;
            }
            std::cout << "TAG ID " << num << ": " << it->second << std::endl;
        }
        std::cout << std::endl;

        std::string cmdString;
        std::string ttmName;
        Command *cmd;
        TTMFile *ttm;
        for (auto it = dump->script.begin(); it != dump->script.end(); ++it) {
            std::cout << "Movie number: " << it->first << " - 0x" << SCRANTIC::BaseFile::hexToString(it->first, std::hex) << std::endl;
            for (size_t pos = 0; pos < it->second.size(); ++pos) {
                num = SCRANTIC::BaseFile::hexToString(pos, std::dec);
                for (size_t j = num.size(); j < 3; ++j) {
                    num = " " + num;
                }

                cmdString = SCRANTIC::BaseFile::commandToString(it->second[pos], true);
                cmd = &(it->second[pos]);
                switch (cmd->opcode) {
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
                    for (size_t j = 0; j < cmd->data.size(); j+=2) {
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
    if (menu != NULL) {
        delete menu;
    }

    delete palette;

    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(fgTexture);
    SDL_DestroyTexture(saveTexture);

    if (audioPlayer != NULL) {
        delete audioPlayer;
    }

    if (res != NULL) {
        delete res;
    }

    std::cout << "-------------- Goodbye from Robinson Crusoe!--------------" << std::endl;
}

void SCRANTIC::Robinson::displaySplash() {
    //display splash
    SDL_Rect splashRect;
    splashRect.x = 0;
    splashRect.y = 0;
    SDL_Texture *splash = static_cast<SCRFile *>(res->getResource("INTRO.SCR"))->getImage(renderer, splashRect);
    SDL_RenderCopy(renderer, splash, &splashRect, &splashRect);
    SDL_RenderPresent(renderer);
}

void SCRANTIC::Robinson::initRenderer(SDL_Renderer *rendererSDL, TTF_Font *font) {
    //renderer and target
    renderer = rendererSDL;
    rendererTarget = SDL_GetRenderTarget(renderer);

    displaySplash();

    // init island
    islandNight = std::rand() % 2;
    islandLarge = std::rand() % 2;
    islandPos = NO_ISLAND;
    islandTrunk = { ISLAND_TEMP_X, ISLAND_TEMP_Y };

    // random pick ocean (0-2)
    oceanRect = { 0, 0, 0, 0 };
    u8 random = std::rand() % 3;
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
    fullRect = {  0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    // width/heigth gets filled in on load
    screenRect = { 0, 0, 0, 0 };

    // create background and foreground texture
    // better pixel format?
    bgTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, fullRect.w, fullRect.h);
    fgTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, fullRect.w, fullRect.h);
    saveTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, fullRect.w, fullRect.h);
    SDL_SetTextureBlendMode(fgTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(saveTexture, SDL_BLENDMODE_BLEND);

    menu = new RobinsonMenu(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    menu->initMenu(res->getMovieList(), font);
}

bool SCRANTIC::Robinson::navigateMenu(SDL_Keycode key) {
    std::string newPage;
    size_t newPos;

    if (menu->navigateMenu(key, newPage, newPos)) {
        ADSFile *ads = static_cast<ADSFile *>(res->getResource(newPage));
        loadMovie(newPage, ads->getMovieNumberFromOrder(newPos));
        renderMenu = false;
        movieRunning = true;
        return true;
    }

    return false;
}

bool SCRANTIC::Robinson::loadMovie(const std::string &adsName, u16 num) {
    ads = static_cast<ADSFile *>(res->getResource(adsName));

    if (!ads) {
        return false;
    }

    script = ads->getFullMovie(num);
    if (script.size()) {
        scriptPos = 0;
    } else {
        return false;
    }

    name = ads->filename;
    menu->setMenuPostion(name, ads->getMoviePosFromNumber(num));
    currentMovie = num;

    labels = ads->getMovieLabels(num);

    resetPlayer();

    return true;
}

void SCRANTIC::Robinson::startMovie() {
    movieRunning = true;
    renderMenu = false;
    delay = 0;
}

void SCRANTIC::Robinson::resetPlayer() {
    //currentMovie = 0;

    for (auto it = ttmScenes.begin(); it != ttmScenes.end(); ++it) {
        delete (*it);
    }

    ttmScenes.clear();

    islandNight = std::rand() % 2;
    u8 random = std::rand() % 3;
    std::string ocean = "OCEAN0"+std::to_string(random)+".SCR";
    oceanTexture = static_cast<SCRFile *>(res->getResource(ocean))->getImage(renderer, oceanRect);
    islandLarge = std::rand() % 2;

    movieRunning = false;
    scrTexture = NULL;
    islandPos= NO_ISLAND;

    lastTTMs.clear();

    delay = 0;
    delayTicks = 0;

    for (int i = 0; i < MAX_IMAGES; ++i) {
        images[i] = NULL;
    }

    SDL_SetRenderTarget(renderer, saveTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, rendererTarget);
}

void SCRANTIC::Robinson::renderBackgroundAtPos(u16 num, i32 x, i32 y, bool raft, bool holiday) {
    SDL_Rect src, dest;
    SDL_Texture *bkg;
    if (!raft && !holiday) {
        bkg = backgroundBMP->getImage(renderer, num, src);
    } else if (raft) {
        bkg = raftBMP->getImage(renderer, num, src);
    } else {
        bkg = holidayBMP->getImage(renderer, num, src);
    }

    dest.x = x;
    dest.y = y;
    dest.w = src.w;
    dest.h = src.h;
    SDL_RenderCopy(renderer, bkg, &src, &dest);
}

void SCRANTIC::Robinson::animateBackground() {
    if (!islandPos) {
        return;
    }

    if (islandLarge) {
        renderBackgroundAtPos((SPRITE_WAVE_L_LEFT + animationCycle/12), WAVE_L_LEFT_X + islandTrunk.x, WAVE_L_LEFT_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_L_MID + ((animationCycle/12 + 1) % 3)), WAVE_L_MID_X + islandTrunk.x, WAVE_L_MID_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_L_RIGHT + ((animationCycle/12 + 2) % 3)), WAVE_L_RIGHT_X + islandTrunk.x, WAVE_L_RIGHT_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_STONE + animationCycle/12), WAVE_STONE_X + islandTrunk.x, WAVE_STONE_Y + islandTrunk.y);
    } else {
        renderBackgroundAtPos((SPRITE_WAVE_LEFT + animationCycle/12), WAVE_LEFT_X + islandTrunk.x, WAVE_LEFT_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_MID + ((animationCycle/12 + 1) % 3)), WAVE_MID_X + islandTrunk.x, WAVE_MID_Y + islandTrunk.y);
        renderBackgroundAtPos((SPRITE_WAVE_RIGHT + ((animationCycle/12 + 2) % 3)), WAVE_RIGHT_X + islandTrunk.x, WAVE_RIGHT_Y + islandTrunk.y);
    }

    //weather is missing

    ++animationCycle;
    if (animationCycle >= 36) {
        animationCycle = 0;
    }
}

void SCRANTIC::Robinson::render() {
    SDL_Rect tmpRect;
    u8 save;

    // first render background
    SDL_SetRenderTarget(renderer, bgTexture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (islandPos > 0) {
        if (islandNight) {
            SDL_RenderCopy(renderer, oceanNightTexture, &oceanRect, &oceanRect);
        } else {
            SDL_RenderCopy(renderer, oceanTexture, &oceanRect, &oceanRect);
        }

        if (islandLarge) {
            renderBackgroundAtPos(SPRITE_L_ISLAND, L_ISLAND_X + islandTrunk.x, L_ISLAND_Y + islandTrunk.y);
            renderBackgroundAtPos(SPRITE_STONE, STONE_X + islandTrunk.x, STONE_Y + islandTrunk.y);
        }

        renderBackgroundAtPos(SPRITE_ISLAND, ISLAND_X + islandTrunk.x, ISLAND_Y + islandTrunk.y);
        renderBackgroundAtPos(SPRITE_TOP_SHADOW, TOP_SHADOW_X + islandTrunk.x, TOP_SHADOW_Y + islandTrunk.y);
        renderBackgroundAtPos(SPRITE_TRUNK, islandTrunk.x, islandTrunk.y);
        renderBackgroundAtPos(SPRITE_TOP, TOP_X + islandTrunk.x, TOP_Y + islandTrunk.y);
        renderBackgroundAtPos(0, RAFT_X + islandTrunk.x, RAFT_Y + islandTrunk.y, true);

        animateBackground();
    } else {
        if (scrTexture != NULL) {
            SDL_RenderCopy(renderer, scrTexture, &screenRect, &screenRect);
        }
    }

    // saved image
    SDL_SetRenderTarget(renderer, saveTexture);
    for (auto it = ttmScenes.begin(); it != ttmScenes.end(); ++it) {
        save = (*it)->needsSave();
        if (save) {
            if (save == 2) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
                SDL_RenderClear(renderer);
            }
            SDL_RenderCopy(renderer, (*it)->savedImage, &fullRect, &fullRect);
        }

        //pre render foreground
        (*it)->renderForeground();
    }
    // background end

    // render everything to screen
    SDL_SetRenderTarget(renderer, rendererTarget);
    SDL_RenderClear(renderer);

    SDL_RenderCopy(renderer, bgTexture, &fullRect, &fullRect);
    SDL_RenderCopy(renderer, saveTexture, &fullRect, &fullRect);

    std::string scrName;
    i16 audio;

    for (auto it = ttmScenes.begin(); it != ttmScenes.end(); ++it) {
        if ((*it)->isClipped()) {
            tmpRect = (*it)->getClipRect();
            SDL_RenderCopy(renderer, (*it)->fg, &tmpRect, &tmpRect);
        } else {
            SDL_RenderCopy(renderer, (*it)->fg, &fullRect, &fullRect);
        }

        audio = (*it)->getSample();
        if (audio != -1) {
            audioPlayer->play(audio);
        }

        scrName = (*it)->getSCRName();
        if (scrName != "") {
            //std::string scrName = (*it)->getSCRName();
            scrTexture = static_cast<SCRFile *>(res->getResource(scrName))->getImage(renderer, screenRect);
            if (scrName == "ISLETEMP.SCR") {
                islandPos = ISLAND_RIGHT;
                islandTrunk.x = ISLAND_TEMP_X;
                islandTrunk.y = ISLAND_TEMP_Y;
            } else if (scrName == "ISLAND2.SCR") {
                islandPos = ISLAND_LEFT;
                islandTrunk.x = ISLAND2_X;
                islandTrunk.y = ISLAND2_Y;
            } else {
                islandPos = NO_ISLAND;
            }

            screenRect.x = 0;
            screenRect.y = 0;
        }
    }

    if (renderMenu) {
        menu->render();
    }
}

void SCRANTIC::Robinson::addTTM(Command cmd) {
    u16 sceneNum;
    i16 repeat;
    if (cmd.opcode == CMD_ADD_INIT_TTM) {
        repeat = 0;
        sceneNum = 0;
    } else {
        sceneNum = cmd.data.at(1);
        repeat = cmd.data.at(2);
        if (repeat) {
            --repeat;
        }
        if (cmd.data.at(2) != 0) {
            std::cout << name << ": TTM Movie with blob " << (i16)cmd.data.at(2) << std::endl;
        }
    }

    //check if a TTM matching this hash already exists - if it does do nothing
    std::pair<u16, u16> hash = std::make_pair(cmd.data.at(0), cmd.data.at(1));
    std::list<TTMPlayer *>::iterator it = ttmScenes.begin();
    while (it != ttmScenes.end()) {
        if ((*it)->getHash() == hash) {
            return;
        } else {
            ++it;
        }
    }

    TTMPlayer *ttm = new TTMPlayer(ads->getResource(cmd.data.at(0)), cmd.data.at(0), sceneNum, repeat, res, images, palette, renderer);

    // this assumes the only relavant action in the init scripts
    // is to load a SCR! all other actions are lost
    if (cmd.opcode == CMD_ADD_INIT_TTM) {
        std::string scrName;

        do {
            ttm->advanceScript();
            scrName = ttm->getSCRName();
            if (scrName != "") {
                scrTexture = static_cast<SCRFile *>(res->getResource(scrName))->getImage(renderer, screenRect);
                if (scrName == "ISLETEMP.SCR") {
                    islandPos = ISLAND_RIGHT;
                    islandTrunk.x = ISLAND_TEMP_X;
                    islandTrunk.y = ISLAND_TEMP_Y;
                } else if (scrName == "ISLAND2.SCR") {
                    islandPos = ISLAND_LEFT;
                    islandTrunk.x = ISLAND2_X;
                    islandTrunk.y = ISLAND2_Y;
                } else {
                    islandPos = NO_ISLAND;
                }

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

size_t SCRANTIC::Robinson::setPosToLabel(std::pair<u16, u16> lastPlayed, size_t next) {
    size_t matches = labels.count(lastPlayed);

    if (!matches) {
        return 0;
    }

    if (matches > 1) {
        std::cout << "==================== multiple ADS Labels " << matches << std::endl;
    }

    if (next == 0) {
        auto it = labels.find(lastPlayed);
        scriptPos = it->second;
    } else {
        std::cout << "==================== multiple ADS Labels jumping to " << next << " label" << std::endl;
        std::pair<std::multimap<std::pair<u16, u16>, size_t>::iterator, std::multimap<std::pair<u16, u16>, size_t>::iterator> ret;
        ret = labels.equal_range(lastPlayed);
        for (auto iteq = ret.first; iteq != ret.second; ++iteq) {
            if (next) {
                next--;
            } else {
                scriptPos = iteq->second;
            }
        }
    }

    return matches;
}

void SCRANTIC::Robinson::runTTMs() {
    TTMPlayer *ttm;
    u16 newDelay = 100;
    u16 oldDelay = delay;
    u32 ticks;

    if (!oldDelay) {
        oldDelay = 100;
    }

    auto it = ttmScenes.begin();
    while (it != ttmScenes.end()) {
        ttm = (*it);
        ticks = ttm->getRemainigDelay(oldDelay);

        if (!ticks) {
            ttm->advanceScript();

            newDelay = ttm->getDelay();

            if (newDelay && (newDelay < delay)) {
                delay = newDelay;
            }
        }

        if (ttm->isFinished()) {
            lastTTMs.push_back(ttm->getHash());
            delete ttm;
            it = ttmScenes.erase(it);
            continue;
        } else {
            ++it;
        }
    }
}

void SCRANTIC::Robinson::advanceScripts() {
    if (ttmScenes.size()) {
        runTTMs();
        if (lastTTMs.size()) {
            for (auto it = lastTTMs.begin(); it != lastTTMs.end(); ++it) {
                advanceADSScript((*it));
            }
            lastTTMs.clear();
        }
    } else {
        advanceADSScript();
    }
}

void SCRANTIC::Robinson::advanceADSScript(std::pair<u16, u16> lastPlayed) {
    if (!script.size() || !movieRunning) {
        return;
    }

    if (scriptPos >= script.size()) {
        movieRunning = false;
        return;
    }

    Command cmd;
    std::vector<Command> pickRandom;

    bool isRandom = false;
    size_t randomPick;
    size_t labelCount = 0;
    size_t labelDone = 0;

    std::pair<u16, u16> hash;
    std::list<TTMPlayer *>::iterator it;

    //on first run go till PLAY_MOVIE
    //next runs check if label for last finished TTM exists and go there
    //otherwise do nothing
    //also: run ADD_TTM_INIT immediately

    if (scriptPos != 0) {
        labelCount = setPosToLabel(lastPlayed);
        if (!labelCount) {
            //std::cout << name << ": no label found for: " << lastPlayed.first << " " << lastPlayed.second << std::endl;
            if (!ttmScenes.size()) {
                movieRunning = false;
            }
            return;
        }
    }

    for (; scriptPos < script.size(); ++scriptPos) {
        cmd = script[scriptPos];

        //std::cout << "ADS Command: " << SCRANTIC::BaseFile::commandToString(cmd) << std::endl;

        switch (cmd.opcode) {
        case CMD_ADD_INIT_TTM:
            addTTM(cmd);
            break;

        case CMD_SKIP_IF_LAST:
            for (size_t i = 0; i < cmd.data.size(); i += 2) {
                if ((cmd.data.at(i) == lastPlayed.first) && (cmd.data.at(i+1) == lastPlayed.second)) {
                    //correct ?
                    while (script[scriptPos++].opcode != CMD_PLAY_MOVIE) {
                        ;
                    }
                    break;
                }
            }
            break;

        case CMD_PLAY_MOVIE:
            labelDone++;
            if (labelCount <= labelDone) {
                runTTMs();
                return;
            } else {
                setPosToLabel(lastPlayed, labelDone);
            }
            break;

        case CMD_ADD_TTM: //TTM Scene ??? Repeat
            if (isRandom) {
                pickRandom.push_back(cmd);
            } else {
                addTTM(cmd);
            }
            break;

        case CMD_KILL_TTM:
            hash = std::make_pair(cmd.data.at(0), cmd.data.at(1));
            it = ttmScenes.begin();
            while (it != ttmScenes.end()) {
                if ((*it)->getHash() == hash) {
                    (*it)->kill();
                    std::cout << "ADS Command: Kill movie " << hash.first << " " << hash.second << std::endl;
                    break;
                } else {
                    ++it;
                }
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
            if (currentMovie == cmd.data.at(0)) {
                break;
            } else {
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

    if (queuedMovie) {
        loadMovie(name, queuedMovie);
        queuedMovie = 0;
        scriptPos = queuedPos;
        advanceADSScript(lastPlayed);
        return;
    }

    movieRunning = false;

    return;
}

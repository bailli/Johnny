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
      compositor(NULL),
      renderer(NULL),
      ads(NULL),
      currentMovie(0),
      movieRunning(false),
      movieFirstRun(true),
      movieLastRun(false),
      delay(0),
      delayTicks(0),
      renderMenu(false) {

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

    if (compositor != NULL) {
        delete compositor;
    }

    delete palette;

    if (audioPlayer != NULL) {
        delete audioPlayer;
    }

    if (res != NULL) {
        delete res;
    }

    std::cout << "-------------- Goodbye from Robinson Crusoe!--------------" << std::endl;
}

void SCRANTIC::Robinson::displaySplash() {
    compositor->displaySplash();
}

void SCRANTIC::Robinson::initRenderer(SDL_Renderer *rendererSDL, TTF_Font *font) {
    renderer = rendererSDL;

    compositor = new RobinsonCompositor(renderer, SCREEN_WIDTH, SCREEN_HEIGHT, res);

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


void SCRANTIC::Robinson::render() {
    compositor->render(ttmScenes.begin(), ttmScenes.end());

    for (auto it = ttmScenes.begin(); it != ttmScenes.end(); ++it) {
        i16 audio = (*it)->getSample();
        if (audio != -1) {
            audioPlayer->play(audio);
        }
    }

    if (renderMenu) {
        menu->render();
    }

}

void SCRANTIC::Robinson::resetPlayer() {
    for (auto scene : ttmScenes) {
        delete scene;
    }

    ttmScenes.clear();

    compositor->reset();

    delay = 0;
    delayTicks = 0;

    for (int i = 0; i < MAX_IMAGES; ++i) {
        images[i] = NULL;
    }
}

void SCRANTIC::Robinson::addTTM(Command cmd) {

    //check if a TTM matching this hash already exists - if it does do nothing
    u16 hash = SCRANTIC::ADSFile::makeHash(cmd.data.at(0), cmd.data.at(1));
    for (auto scene : ttmScenes) {
        if (scene->getHash() == hash) {
            return;
        }
    }

    u16 sceneNum = 0;
    i16 repeat = 0;

    if (cmd.opcode != CMD_ADD_INIT_TTM) {
        sceneNum = cmd.data.at(1);
        repeat = cmd.data.at(2);

        if (repeat) {
            --repeat;
        }
    } else {
        if (ads->getResource(cmd.data.at(0)) == "WOULDBE.TTM") {
            sceneNum = 3;
        }
    }

    TTMPlayer *ttm = new TTMPlayer(ads->getResource(cmd.data.at(0)), cmd.data.at(0), sceneNum, repeat, res, images, palette, renderer);

    if (cmd.opcode != CMD_ADD_INIT_TTM) {
        ttmScenes.push_back(ttm);
        return;
    }

    // this assumes the only relavant action in the init scripts
    // is to load a SCR! all other actions are lost
    std::string scrName;

    do {
        ttm->advanceScript();
        scrName = ttm->getSCRName();
        if (scrName != "") {
            compositor->setScreen(scrName);
        }
    } while (!ttm->isFinished());

    delete ttm;
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
            lastHashes.push_back(ttm->getHash());
            delete ttm;
            it = ttmScenes.erase(it);
            continue;
        } else {
            ++it;
        }
    }
}

bool SCRANTIC::Robinson::loadMovie(const std::string &adsName, u16 num) {
    ads = static_cast<ADSFile *>(res->getResource(adsName));

    if (!ads) {
        return false;
    }

    adsFileName = ads->filename;
    menu->setMenuPostion(adsFileName, ads->getMoviePosFromNumber(num));
    currentMovie = num;

    std::cout << "Playing ADS movie " << ads->tagList[num] << std::endl;

    movieRunning = false;
    movieFirstRun = true;
    movieLastRun = false;
    lastHashes.clear();

    resetPlayer();

    return true;
}

void SCRANTIC::Robinson::startMovie() {
    movieRunning = true;
    renderMenu = false;
}

void SCRANTIC::Robinson::advanceScripts() {
    if (!movieRunning) {
        return;
    }

    if (movieFirstRun) {
        movieFirstRun = false;
        runADSBlock(false, currentMovie, 0, 0);
        return;
    }

    if (!ttmScenes.empty()) {
        runTTMs();
        if (lastHashes.size() && !movieLastRun) {
            auto it = lastHashes.begin();
            while (it != lastHashes.end()) {
                u16 hash = *it;
                size_t countAfter = ads->getLabelCountAfter(currentMovie, hash);

                for (size_t i = 0; i < countAfter; ++i) {
                    runADSBlock(false, currentMovie, hash, i);
                }
                ++it;
            }
            lastHashes.clear();
        }
        return;
    }

    std::cout << adsFileName << ": Finished ADS Movie: " << currentMovie << std::endl;

    if (movieQueue.empty()) {
        movieRunning = false;
        return;
    }

    std::pair<u16, bool> nextItem = *movieQueue.begin();
    bool skipToPlayAds = nextItem.second;
    currentMovie = nextItem.first;

    movieQueue.erase(movieQueue.begin());

    runADSBlock(false, currentMovie, 0, 0, skipToPlayAds);
}

void SCRANTIC::Robinson::runADSBlock(bool togetherWith, u16 movie, u16 hash, u16 num, bool skipToPlayAds) {
    std::vector<Command> block;

    if (hash == 0) {
        block = ads->getInitialBlock(movie);
    } else if (togetherWith) {
        block = ads->getBlockTogetherWithMovie(movie, hash, num);
    } else {
        block = ads->getBlockAfterMovie(movie, hash, num);
    }

    bool done = false;
    bool found;
    u16 lastHash = 0;
    bool skipToPlayMovie = false;
    bool isRandom = false;
    size_t randomChoice;

    std::vector<Command> randomBlock;
    std::vector<Command> ttmsToAdd;
    std::vector<u16> ttmsToAddHashes;

    for (auto cmd : block) {

        if (skipToPlayAds) {
            if (cmd.opcode == CMD_PLAY_ADS_MOVIE) {
                skipToPlayAds = false;
            }
            continue;
        }

        if (skipToPlayMovie) {
            if (cmd.opcode == CMD_PLAY_MOVIE) {
                skipToPlayMovie = false;
            }
            continue;
        }

        switch (cmd.opcode) {

        case CMD_ADD_INIT_TTM:
            addTTM(cmd);
            break;

        case CMD_SKIP_IF_PLAYED:
            skipToPlayMovie = false;
            for (size_t i = 0; i < cmd.data.size(); i += 2) {
                lastHash = SCRANTIC::ADSFile::makeHash(cmd.data.at(i), cmd.data.at(i+1));
                if (std::find(lastHashes.begin(), lastHashes.end(), lastHash) != lastHashes.end()) {
                    skipToPlayMovie = true;
                    break;
                }
                if (std::find(ttmsToAddHashes.begin(), ttmsToAddHashes.end(), lastHash) != ttmsToAddHashes.end()) {
                    skipToPlayMovie = true;
                    break;
                }
            }
            break;

        case CMD_ONLY_IF_PLAYED:
            skipToPlayMovie = true;
            for (size_t i = 0; i < cmd.data.size(); i += 2) {
                lastHash = SCRANTIC::ADSFile::makeHash(cmd.data.at(i), cmd.data.at(i+1));
                if (std::find(ttmsToAddHashes.begin(), ttmsToAddHashes.end(), lastHash) != ttmsToAddHashes.end()) {
                    skipToPlayMovie = false;
                    break;
                }
            }
            break;

        case CMD_PLAY_MOVIE:
            done = true;
            break;

        case CMD_ADD_TTM: //TTM Scene ??? Repeat
            if (isRandom) {
                for (u16 j = 0; j < cmd.data.at(3); j++) {
                    randomBlock.push_back(cmd);
                }
            } else {
                ttmsToAdd.push_back(cmd);
                ttmsToAddHashes.push_back(SCRANTIC::ADSFile::makeHash(cmd.data.at(0), cmd.data.at(1)));
            }
            break;

        case CMD_ZERO_CHANCE:
            if (!isRandom) {
                std::cerr << "ERROR: CMD_ZERO_CHANCE found outside of random block!" << std::endl;
            }
            for (u16 j = 0; j < cmd.data.at(0); j++) {
                randomBlock.push_back(cmd);
            }
            break;

        case CMD_RANDOM_START:
            isRandom = true;
            randomBlock.clear();
            break;

        case CMD_RANDOM_END:
            isRandom = false;
            randomChoice = std::rand() % randomBlock.size();
            if (randomBlock[randomChoice].opcode != CMD_ZERO_CHANCE) {
                ttmsToAdd.push_back(randomBlock[randomChoice]);
                ttmsToAddHashes.push_back(SCRANTIC::ADSFile::makeHash(randomBlock[randomChoice].data.at(0), randomBlock[randomChoice].data.at(1)));
            }
            break;

        case CMD_KILL_TTM:
            lastHash = SCRANTIC::ADSFile::makeHash(cmd.data.at(0), cmd.data.at(1));
            found = false;
            for (auto scene : ttmScenes) {
                if (lastHash == scene->getHash()) {
                    scene->kill();
                    found = true;
                    break;
                }
            }

            if (!found) {
                std::cout << "ADS Command: Kill movie not found ! " << (u16)cmd.data.at(0) << " " << cmd.data.at(1) << std::endl;
            }
            break;

        case CMD_PLAY_ADS_MOVIE:
            if (currentMovie != cmd.data.at(0)) {
                movieQueue.push_front(std::make_pair(currentMovie, true));
                movieQueue.push_front(std::make_pair(cmd.data.at(0), false));
                movieLastRun = true;
                return;
            }
            break;

        case CMD_SET_SCENE:
            std::cout << "Play ADS Movie: " << cmd.name << std::endl;
            break;

        case CMD_END_SCRIPT:
            movieLastRun = true;
            break;

        default:
            std::cout << "ADS Command: " << SCRANTIC::BaseFile::commandToString(cmd, true) << std::endl;
            break;
        }

        if (done) {
            break;
        }
    }

    for (auto ttm : ttmsToAdd) {
        addTTM(ttm);
        lastHash = SCRANTIC::ADSFile::makeHash(ttm.data.at(0), ttm.data.at(1));
        size_t count = ads->getLabelCountTogether(movie, lastHash);
        for (size_t j = 0; j < count; ++j) {
            runADSBlock(true, movie, lastHash, j);
        }
    }

    runTTMs();
}

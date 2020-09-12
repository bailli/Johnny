#include "RobinsonWalker.h"
#include "RobinsonWalkerConst.h"

#include <algorithm>
#include <iterator>

#define IMGSLOT " 0009"

SCRANTIC::RobinsonWalker::RobinsonWalker(const std::string& path, bool readFromFiles)
    : johnWalkTTM(NULL) {
    if (readFromFiles) {
        johnWalkTTM = new TTMFile(path + "JOHNWALK.TTM");
    } else {
        johnWalkTTM = createJohnWalkTTM(path);
    }
    splitTurnAnimation();
}

SCRANTIC::RobinsonWalker::~RobinsonWalker() {
    if (johnWalkTTM) {
        delete johnWalkTTM;
    }
}

void SCRANTIC::RobinsonWalker::splitTurnAnimation() {
    for (size_t i = 0; i < WALKER_MAX_POS; ++i) {
        std::vector<Command> turnList = johnWalkTTM->getFullScene(walkerTurnTags[i]);

        std::map<i8, std::vector<Command> > posList;

        i8 direction = -1;
        size_t pos = 0;

        for (; pos < 3; ++pos) {
            posList[direction].push_back(turnList[pos]);
        }
        --pos;

        for (direction = 0; direction < WALKER_MAX_POS; ++direction) {
            do {
                posList[direction].push_back(turnList[++pos]);
            } while (turnList[pos].opcode != CMD_UPDATE);
        }
        turnFrames[i] = posList;
    }
}

std::vector<SCRANTIC::Command> SCRANTIC::RobinsonWalker::getTurnFrames(i8 pos, i8 start, i8 end) {
    std::vector<Command> commandList;
    commandList.insert(commandList.end(), turnFrames[pos][-1].begin(), turnFrames[pos][-1].end());

    int distClockwise = end - start;
    if (distClockwise < 0) {
        distClockwise += 8;
    }

    int distCounterClockwise = start - end;
    if (distCounterClockwise < 0) {
        distCounterClockwise += 8;
    }

    bool clockwise = distClockwise <= distCounterClockwise;
    std::cout << "Clockwise " << clockwise << std::endl;

    i8 i = start;
    bool lastFrame = false;
    do {
        commandList.insert(commandList.end(), turnFrames[pos][i].begin(), turnFrames[pos][i].end());
        i = clockwise ? i + 1 : i - 1;

        if (i >= WALKER_MAX_FACE) {
            i = 0;
        }
        if (i < 0) {
            i = WALKER_MAX_FACE - 1;
        }

        if (lastFrame) {
            break;
        }

        if (i == end) {
            lastFrame = true;
        }
    } while (true);

    return commandList;
}

std::vector<SCRANTIC::Command> SCRANTIC::RobinsonWalker::getTransitionAndNextMovie(const std::string& adsName, u16 currentMovie, std::string &nextAds, u16 &nextMovie) {
    i8 currentPos = movieData[adsName][currentMovie].endPos;
    i8 nextPos = getRandomNextPos(currentPos);

    // choose between wait, stand.ads or any movie ?
    size_t choice = std::rand() % 3;

    std::cout << "Choice " << choice << std::endl;

    if (choice == 0) { // wait
        //placeholder
        getRandomNextMovieAtPos(currentPos, nextAds, nextMovie);
    } else if (choice == 1) { // stand.ads
        getRandomNextStandMovieAtPos(currentPos, nextAds, nextMovie);
    } else {
        getRandomNextMovieAtPos(currentPos, nextAds, nextMovie);
    }

    return getWalkScript(currentPos, movieData[adsName][currentMovie].endFacing, movieData[nextAds][nextMovie].startPos, movieData[nextAds][nextMovie].startFacing);
}

std::vector<SCRANTIC::Command> SCRANTIC::RobinsonWalker::getWalkScript(i8 startPos, i8 startFacing, i8 endPos, i8 endFacing) {
    std::vector<SCRANTIC::Command> commandList;

    if (startPos == endPos) {
        return getTurnFrames(startPos, startFacing, endFacing);
    }

    i8 xFacing = walkerXtoYFace[startPos][endPos][0];
    std::vector<SCRANTIC::Command> turnList = getTurnFrames(startPos, startFacing, xFacing);
    commandList.insert(commandList.end(), turnList.begin(), turnList.end());

    i8 tag = getXtoYTagNum(startPos, endPos);
    std::vector<SCRANTIC::Command> walkList = johnWalkTTM->getFullScene(tag);
    commandList.insert(commandList.end(), walkList.begin(), walkList.end());

    i8 yFacing = walkerXtoYFace[startPos][endPos][1];
    turnList = getTurnFrames(endPos, yFacing, endFacing);
    commandList.insert(commandList.end(), turnList.begin(), turnList.end());

    return commandList;
}

bool SCRANTIC::RobinsonWalker::canContinueFromCurrentState(const std::string &adsName, u16 currentMovie) {
    return (movieData[adsName][currentMovie].endPos != -1) && (movieData[adsName][currentMovie].endFacing != -1);
}

i8 SCRANTIC::RobinsonWalker::getRandomNextPos(i8 pos) {
    std::vector<i8> positions;
    for (int i = 0; i < WALKER_MAX_POS; ++i) {
        if (walkerXtoYTags[pos][i] != -1) {
            positions.push_back(walkerXtoYTags[pos][i]);
        }
    }

    size_t choice = std::rand() % positions.size();
    return positions[choice];
}

void SCRANTIC::RobinsonWalker::getRandomNextStandMovieAtPos(i8 pos, std::string& adsName, u16& movie) {
    std::vector<u16> movies;
    for (auto it = movieData["STAND.ADS"].begin(); it != movieData["STAND.ADS"].end(); ++it) {
        if (it->second.startPos == pos) {
            movies.push_back(it->first);
        }
    }

    size_t choice = std::rand() % movies.size();
    movie = movies[choice];
    adsName = "STAND.ADS";
}

void SCRANTIC::RobinsonWalker::getRandomNextMovieAtPos(i8 pos, std::string& adsName, u16& movie) {
    std::vector<std::pair<std::string, u16> > movies;
    for (auto it = movieData.begin(); it != movieData.end(); ++it) {
        for (auto adsIt = it->second.begin(); adsIt != it->second.end(); ++adsIt) {
            if (adsIt->second.startPos == pos) {
                movies.push_back({it->first, adsIt->first});
            }
        }
    }

    size_t choice = std::rand() % movies.size();
    movie = std::get<1>(movies[choice]);
    adsName = std::get<0>(movies[choice]);
}

i8 SCRANTIC::RobinsonWalker::getTurnTagNum(i8 pos) {
    if ((pos < 0) || (pos >= WALKER_MAX_POS)) {
        return -1;
    }

    return walkerTurnTags[pos];
}

i8 SCRANTIC::RobinsonWalker::getWaitTagNum(i8 pos) {
    if ((pos < 0) || (pos >= WALKER_MAX_POS)) {
        return -1;
    }

    return walkerWaitTags[pos];
}

i8 SCRANTIC::RobinsonWalker::getXtoYTagNum(i8 startPos, i8 endPos) {
    if ((startPos < 0) || (startPos >= WALKER_MAX_POS) || (endPos < 0) || (endPos >= WALKER_MAX_POS)) {
        return -1;
    }

    return walkerXtoYTags[startPos][endPos];
}

SCRANTIC::TTMFile * SCRANTIC::RobinsonWalker::createJohnWalkTTM(const std::string& path) {
    std::map<u16, std::string> tagList;

    tagList[1] = "A to E";
    tagList[2] = "A to F";
    tagList[3] = "A to C";
    tagList[4] = "A to B";
    tagList[5] = "A turn";
    tagList[6] = "A wait";
    tagList[7] = "B to A";
    tagList[8] = "B to C";
    tagList[9] = "B turn";
    tagList[10] = "B wait";
    tagList[11] = "C to A";
    tagList[12] = "C to B";
    tagList[13] = "C to D";
    tagList[14] = "C to E";
    tagList[15] = "C to F";
    tagList[16] = "C turn";
    tagList[17] = "C wait";
    tagList[18] = "D to C";
    tagList[19] = "D to E";
    tagList[20] = "D to F";
    tagList[21] = "D turn";
    tagList[22] = "D wait";
    tagList[23] = "E to A";
    tagList[24] = "E to C";
    tagList[25] = "E to D";
    tagList[26] = "E to F";
    tagList[27] = "E turn";
    tagList[28] = "E wait";
    tagList[29] = "F to A";
    tagList[30] = "F to C";
    tagList[31] = "F to D";
    tagList[32] = "F to E";
    tagList[33] = "F turn";
    tagList[34] = "F wait";

    std::ifstream in;
    in.open(path + "SCRANTIC.SCR", std::ios::binary | std::ios::in);
    in.unsetf(std::ios::skipws);

    if (!in.is_open()) {
        std::cerr << "RobinsonWalker: Error opening " << path << "SCRANTIC.SCR" << std::endl;
        return NULL;
    }

    size_t pos = 0x188EA;
    in.seekg(pos, std::ios_base::beg);

    u16 num;
    i16 x, y;

    std::stringstream output;

    output << "# SCRANTIC TTM file" << std::endl;
    output << "VERSION" << std::endl;
    output << "1.00" << std::endl << std::endl;

    output << "TAGS" << std::endl;
    for (auto it = tagList.begin(); it != tagList.end(); ++it) {
        output << BaseFile::hexToString(it->first, std::hex, 4) << " " << it->second << std::endl;
    }

    u16 id = 1;

    output << std::endl << "SCRIPT" << std::endl;
    output << "SCREEN ISLETEMP.SCR" << std::endl;
    output << "PALSLOT 0000" << std::endl;
    output << "PALETTE JOHNCAST.PAL" << std::endl;
    output << "UPDATE" << std::endl;
    output << "# new scene" << std::endl;
    output << "# " << tagList[id] << std::endl;
    output << "SCENE 0001" << std::endl;
    output << "IMGSLOT" << IMGSLOT << std::endl;
    output << "BITMAP JOHNWALK.BMP" << std::endl;

    while (pos < 0x19456) {
        BaseFile::readUintLE(&in, num);
        BaseFile::readUintLE(&in, x);
        BaseFile::readUintLE(&in, y);

        if ((x == 0) && (y == 0) && (num == 0)) {
            if (id < 34) {
                id++;

                output << "# new scene" << std::endl;
                output << "# " << tagList[id] << std::endl;
                output << "SCENE " << BaseFile::hexToString(id, std::hex, 4) << std::endl;
                output << "IMGSLOT" << IMGSLOT << std::endl;
                output << "BITMAP JOHNWALK.BMP" << std::endl;

            }
            continue;
        }

        std::string line;

        Command c;
        if (num & 0x8000) {
            line = "SPRITEHINV";
        } else {
            line = "SPRITE";
        }

        line += " " + BaseFile::hexToString(x, std::hex, 4);
        line += " " + BaseFile::hexToString(y, std::hex, 4);
        line += " " + BaseFile::hexToString(num & 0x7FFF, std::hex, 4);
        line += IMGSLOT;

        output << "CLEAR 0000" << std::endl;
        output << line << std::endl;
        output << "UPDATE" << std::endl;

        pos += 6;
    }

    in.close();

    std::string ttmName = "JOHNWALK.TTM";
    BaseFile::writeFile(output.str(), ttmName, path);

    return new TTMFile(path + ttmName);
}


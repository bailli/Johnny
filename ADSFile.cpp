#include "ADSFile.h"

SCRANTIC::ADSFile::ADSFile(const std::string &name, v8 &data)
    : CompressedBaseFile(name) {

    initMnemonics();
    parseFile(data);

    parseRawScript();
    findLabels();
}

SCRANTIC::ADSFile::ADSFile(const std::string &filename)
    : CompressedBaseFile(filename) {

    initMnemonics();

    std::ifstream in;

    in.open(filename, std::ios::in);
    if (!in.is_open()) {
        std::cerr << "ADSFile: Could not open " << filename << std::endl;
        return;
    }

    u16 i, j, k, l;

    std::string line;
    std::string tag;
    std::string mnemonic;
    u16 id, opcode;
    int count;

    std::string mode;

    while (getline(in, line)) {
        if (line.substr(0, 1) == "#" || line == "") {
            continue;
        }

        if (line == "VERSION") {
            getline(in, line);
            version = line;
            continue;
        }

        if (line == "RESOURCES") {
            mode = line;
            continue;
        }

        if (line == "TAGS") {
            mode = line;
            continue;
        }

        if (line == "SCRIPT") {
            mode = line;
            continue;
        }

        if (mode == "RESOURCES") {
            std::istringstream iss(line);
            if (!(iss >> std::hex >> id)) {
                break;
            }
            tag = line.substr(line.find(" ")+1);
            resList.insert({id, tag});
        } else if (mode == "TAGS") {
            std::istringstream iss(line);
            if (!(iss >> std::hex >> id)) {
                break;
            }
            tag = line.substr(line.find(" ")+1);
            tagList.insert({id, tag});
        }  else if (mode == "SCRIPT") {
            mnemonic = line.substr(0, line.find(" "));
            opcode = getOpcodeFromMnemonic(mnemonic);
            count = getParamCount(opcode);

            if (opcode != CMD_SET_SCENE) {
                writeUintLE(rawScript, opcode);
            }

            std::istringstream iss(line);
            switch (count) {
            case 1:
                iss >> mnemonic >> std::hex >> i;
                writeUintLE(rawScript, i);
                break;
            case 2:
                iss >> mnemonic >> std::hex >> i >> j;
                writeUintLE(rawScript, i);
                writeUintLE(rawScript, j);
                break;
            case 3:
                iss >> mnemonic >> std::hex >> i >> j >> k;
                writeUintLE(rawScript, i);
                writeUintLE(rawScript, j);
                writeUintLE(rawScript, k);
                break;
            case 4:
                iss >> mnemonic >> std::hex >> i >> j >> k >> l;
                writeUintLE(rawScript, i);
                writeUintLE(rawScript, j);
                writeUintLE(rawScript, k);
                writeUintLE(rawScript, l);
                break;
            default:
                break;
            }
        }
    }

    in.close();

    parseRawScript();
    findLabels();
}

void SCRANTIC::ADSFile::parseFile(v8 &data) {
    v8::iterator it = data.begin();

    assertString(it, "VER:");

    readUintLE(it, verSize);
    version = readString(it, verSize-1);

    assertString(it, "ADS:");

    readUintLE(it, resScrTagSize);
    readUintLE(it, magic);

    assertString(it, "RES:");

    readUintLE(it, resSize);
    readUintLE(it, resCount);

    u16 id;
    std::string desc;

    for (u16 i = 0; i < resCount; ++i) {
        readUintLE(it, id);
        desc = readString(it);
        resList.insert(std::pair<u16, std::string>(id, desc));
    }

    assertString(it, "SCR:");

    if (!handleDecompression(data, it, rawScript)) {
        return;
    }

    std::advance(it, compressedSize);

    assertString(it, "TAG:");

    readUintLE(it, tagSize);
    readUintLE(it, tagCount);

    for (u16 i = 0; i < tagCount; ++i) {
        readUintLE(it, id);
        desc = readString(it);
        tagList.insert(std::pair<u16, std::string>(id, desc));
    }
}

void SCRANTIC::ADSFile::parseRawScript() {
    if (!rawScript.size()) {
        return;
    }

    script.clear();

    v8::iterator it = rawScript.begin();
    u16 opcode, word, listOpcode;

    u16 maxTagId = tagList.rbegin()->first;
    u16 currentMovie;

    while (it != rawScript.end()) {
        readUintLE(it, opcode);
        size_t length = getParamCount(opcode);

        Command c;
        c.opcode = opcode;

        if (opcode <= maxTagId) {
            c.opcode = CMD_SET_SCENE;
            c.data.push_back(opcode);
            c.name = tagList.find(opcode)->second;
            currentMovie = opcode;
            script[currentMovie].push_back(c);
            continue;
        }

        for (size_t i = 0; i < length; ++i) {
            readUintLE(it, word);
            c.data.push_back(word);
        }

        if (opcode == CMD_AFTER_SCENE || opcode == CMD_SKIP_IF_PLAYED) {
            if (opcode == CMD_AFTER_SCENE) {
                listOpcode = CMD_OR_AFTER;
            } else {
                listOpcode = CMD_OR_SKIP;
            }

            readUintLE(it, word);
            while (word == listOpcode) {
                readUintLE(it, word);
                if (word != opcode) {
                    std::cerr << filename << ": Error parsing label list "<< word << std::endl;
                    break;
                }
                readUintLE(it, word);
                c.data.push_back(word);
                readUintLE(it, word);
                c.data.push_back(word);
                readUintLE(it, word);
            }
            it -= 2;
        }

        script[currentMovie].push_back(c);
    }
}

u16 SCRANTIC::ADSFile::getMovieNumberFromOrder(size_t pos) {
    size_t count = 0;

    for (auto it = tagList.begin(); it != tagList.end(); ++it) {
        if (pos == count) {
            return it->first;
        }
        ++count;
    }

    return 0;
}

size_t SCRANTIC::ADSFile::getMoviePosFromNumber(u16 number) {
    size_t count = 0;

    for (auto it = tagList.begin(); it != tagList.end(); ++it) {
        if (number == it->first) {
            return count;
        }
        ++count;
    }

    return 0;
}



void SCRANTIC::ADSFile::findLabels() {
    u16 currentMovie;
    u16 currentHash = 0;
    size_t playCount = 1;

    for (auto movieIt = script.begin(); movieIt != script.end(); ++movieIt) {
        currentMovie = movieIt->first;
        playCount = 1;
        for (size_t pos = 0; pos < movieIt->second.size(); ++pos) {
            Command c = movieIt->second[pos];
            if (c.opcode == CMD_AFTER_SCENE) {
                if (playCount != 0) {
                    std::cout << "PlayCount mismatch!" << std::endl;
                }
                playCount = 1;
                for (size_t i = 0; i < c.data.size(); i += 2) {
                    currentHash = makeHash(c.data.at(i), c.data.at(i+1));
                    labelsAfter[currentMovie][currentHash].push_back(pos);
                }
            } else if (c.opcode == CMD_PLAY_MOVIE) {
                playCount--;
            } else if ((c.opcode == CMD_SKIP_IF_PLAYED) || (c.opcode == CMD_ONLY_IF_PLAYED)) {
                playCount++;
            } else if ((c.opcode == CMD_ONLY_IF_PLAYED) && (playCount == 0)) {
                playCount = 1;
                currentHash = makeHash(c.data.at(0), c.data.at(1));
                labelsTogether[currentMovie][currentHash].push_back(pos);
            }
        }
    }
}

size_t SCRANTIC::ADSFile::getLabelCountAfter(u16 movie, u16 hash) {
    if (labelsAfter.find(movie) == labelsAfter.end()) {
        return 0;
    }

    if (labelsAfter[movie].find(hash) == labelsAfter[movie].end()) {
        return 0;
    }

    return labelsAfter[movie][hash].size();
}

size_t SCRANTIC::ADSFile::getLabelCountTogether(u16 movie, u16 hash) {
    if (labelsTogether.find(movie) == labelsTogether.end()) {
        return 0;
    }

    if (labelsTogether[movie].find(hash) == labelsTogether[movie].end()) {
        return 0;
    }

    return labelsTogether[movie][hash].size();
}

std::vector<SCRANTIC::Command> SCRANTIC::ADSFile::getInitialBlock(u16 movie) {
    std::vector<Command> block;
    size_t playCount = 1;

    for (size_t pos = 1; pos < script[movie].size(); ++pos) {
        Command c = script[movie][pos];
        if ((c.opcode == CMD_SKIP_IF_PLAYED) || (c.opcode == CMD_ONLY_IF_PLAYED)) {
            playCount++;
        }

        if (c.opcode == CMD_PLAY_MOVIE) {
            playCount--;
        }

        block.push_back(c);

        if (playCount == 0) {
            break;
        }
    }

    return block;
}

std::vector<SCRANTIC::Command> SCRANTIC::ADSFile::getBlockAfterMovie(u16 movie, u16 hash, u16 num) {
    std::vector<Command> block;

    if (num >= getLabelCountAfter(movie, hash)) {
        return block;
    }

    size_t playCount = 1;
    size_t pos = labelsAfter[movie][hash][num] + 1;

    for (; pos < script[movie].size(); ++pos) {
        Command c = script[movie][pos];
        if ((c.opcode == CMD_SKIP_IF_PLAYED) || (c.opcode == CMD_ONLY_IF_PLAYED)) {
            playCount++;
        }

        if (c.opcode == CMD_PLAY_MOVIE) {
            playCount--;
        }

        block.push_back(c);

        if (playCount == 0) {
            break;
        }
    }

    return block;
}

std::vector<SCRANTIC::Command> SCRANTIC::ADSFile::getBlockTogetherWithMovie(u16 movie, u16 hash, u16 num) {
    std::vector<Command> block;

    if (num >= getLabelCountTogether(movie, hash)) {
        return block;
    }

    size_t playCount = 1;
    size_t pos = labelsTogether[movie][hash][num] + 1;

    for (; pos < script[movie].size(); ++pos) {
        Command c = script[movie][pos];
        if (c.opcode == CMD_SKIP_IF_PLAYED) {
            playCount++;
        }

        if (c.opcode == CMD_PLAY_MOVIE) {
            playCount--;
        }

        block.push_back(c);

        if (playCount == 0) {
            break;
        }
    }

    return block;
}

v8 SCRANTIC::ADSFile::repackIntoResource() {
    std::string strings[5] = { "VER:", "ADS:", "RES:", "SCR:", "TAG:" };

    compressionFlag = 2;
    v8 compressedData = LZCCompress(rawScript);
    uncompressedSize = rawScript.size();
    compressedSize = compressedData.size() + 5;

    verSize = version.size() + 1;
    magic = 0x8000;

    resSize = 2;
    resCount = resList.size();
    for (auto it = resList.begin(); it != resList.end(); ++it) {
        resSize += 2 + it->second.size() + 1;
    }

    tagCount = tagList.size();
    tagSize = 2;
    for (auto it = tagList.begin(); it != tagList.end(); ++it) {
        tagSize += 2 + it->second.size() + 1;
    }

    resScrTagSize = tagSize + resSize + compressedSize + 24;

    v8 rawData(strings[0].begin(), strings[0].end());
    BaseFile::writeUintLE(rawData, verSize);
    std::copy(version.begin(), version.end(), std::back_inserter(rawData));
    rawData.push_back(0);

    std::copy(strings[1].begin(), strings[1].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, resScrTagSize);
    BaseFile::writeUintLE(rawData, magic);

    std::copy(strings[2].begin(), strings[2].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, resSize);
    BaseFile::writeUintLE(rawData, resCount);
    for (auto it = resList.begin(); it != resList.end(); ++it) {
        BaseFile::writeUintLE(rawData, it->first);
        std::copy(it->second.begin(), it->second.end(), std::back_inserter(rawData));
        rawData.push_back(0);
    }

    std::copy(strings[3].begin(), strings[3].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, compressedSize);
    BaseFile::writeUintLE(rawData, compressionFlag);
    BaseFile::writeUintLE(rawData, uncompressedSize);
    std::copy(compressedData.begin(), compressedData.end(), std::back_inserter(rawData));
    compressedSize -= 5;

    std::copy(strings[4].begin(), strings[4].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, tagSize);
    BaseFile::writeUintLE(rawData, tagCount);

    for (auto it = tagList.begin(); it != tagList.end(); ++it) {
        BaseFile::writeUintLE(rawData, it->first);
        std::copy(it->second.begin(), it->second.end(), std::back_inserter(rawData));
        rawData.push_back(0);
    }

    return rawData;
}

std::string SCRANTIC::ADSFile::getResource(u16 num) {
    auto it = resList.find(num);
    if (it == resList.end()) {
        return "";
    } else {
        return it->second;
    }
}

void SCRANTIC::ADSFile::saveFile(const std::string &path) {
    std::stringstream output;

    output << "# SCRANTIC ADS file" << std::endl;
    output << "VERSION" << std::endl;
    output << version << std::endl << std::endl;

    output << "RESOURCES" << std::endl;
    for (auto it = resList.begin(); it != resList.end(); ++it) {
        output << hexToString(it->first, std::hex, 4) << " " << it->second << std::endl;
    }

    output << std::endl << "TAGS" << std::endl;
    for (auto it = tagList.begin(); it != tagList.end(); ++it) {
        output << hexToString(it->first, std::hex, 4) << " " << it->second << std::endl;
    }

    output << std::endl << "SCRIPT" << std::endl;

    v8::iterator it = rawScript.begin();
    u16 opcode;
    u8 length;
    u16 word;

    while (it != rawScript.end()) {
        readUintLE(it, opcode);
        Command command;

        if (opcode <= 0x100) {
            output << "# new movie" << std::endl;
            command.opcode = CMD_SET_SCENE;
            command.data.push_back(opcode);
        } else {
            length = getParamCount(opcode);
            command.opcode = opcode;
            for (u8 i = 0; i < length; ++i) {
                readUintLE(it, word);
                command.data.push_back(word);
            }
        }

        output << getMnemoic(command) << std::endl;
    }

    writeFile(output.str(), filename, path);
}


void SCRANTIC::ADSFile::initMnemonics() {
    // fake
    mnemonics.insert({CMD_SET_SCENE, "SCENE"});
    //no params
    mnemonics.insert({CMD_OR_SKIP, "OR"});
    mnemonics.insert({CMD_OR_AFTER, "AND"});
    mnemonics.insert({CMD_PLAY_MOVIE, "PLAYMOVIE"});
    mnemonics.insert({CMD_UNK_1520, "UNK1520"});
    mnemonics.insert({CMD_RANDOM_START, "RANDSTART"});
    mnemonics.insert({CMD_RANDOM_END, "RANDEND"});
    mnemonics.insert({CMD_UNK_LABEL, "UNKLABEL"});
    mnemonics.insert({CMD_UNK_F010, "UNKF010"});
    mnemonics.insert({CMD_END_SCRIPT, "ENDSCRIPT"});
    //1 param
    mnemonics.insert({CMD_PLAY_ADS_MOVIE, "PLAYADS"});
    mnemonics.insert({CMD_ZERO_CHANCE, "ZEROCHANCE"});
    //2 params
    mnemonics.insert({CMD_UNK_1070, "UNK1070"});
    mnemonics.insert({CMD_ADD_INIT_TTM, "INITTTM"});
    mnemonics.insert({CMD_AFTER_SCENE, "CONTINUEAFTER"});
    mnemonics.insert({CMD_SKIP_IF_PLAYED, "SKIPAFTER"});
    mnemonics.insert({CMD_ONLY_IF_PLAYED, "TOGETHERWITH"});
    //3 params
    mnemonics.insert({CMD_KILL_TTM, "KILLTTM"});
    //4 params
    mnemonics.insert({CMD_ADD_TTM, "ADDTTM"});
}


std::string SCRANTIC::ADSFile::getMnemoic(SCRANTIC::Command c) {
    std::string result = mnemonics[c.opcode];

    int count = getParamCount(c.opcode);

    if (!count) {
        return result;
    }

    for (int i = 0; i < count; ++i) {
        std::string data = hexToString(c.data.at(i), std::hex, 4);
        result += " " + data;
    }

    return result;
}

u16 SCRANTIC::ADSFile::getOpcodeFromMnemonic(std::string &mnemonic) {
    for (auto it = mnemonics.begin(); it != mnemonics.end(); ++it) {
        if (it->second == mnemonic) {
            return it->first;
        }
    }
    return 0;
}


int SCRANTIC::ADSFile::getParamCount(u16 opcode) {
    switch (opcode) {
    case CMD_OR_SKIP:
    case CMD_OR_AFTER:
    case CMD_PLAY_MOVIE:
    case CMD_UNK_1520:
    case CMD_RANDOM_START:
    case CMD_RANDOM_END:
    case CMD_UNK_F010:
    case CMD_END_SCRIPT:
        return 0;
    case CMD_SET_SCENE: // beware fake
    case CMD_PLAY_ADS_MOVIE:
    case CMD_ZERO_CHANCE:
        return 1;
    case CMD_UNK_1070:
    case CMD_ADD_INIT_TTM:
    case CMD_AFTER_SCENE:
    case CMD_SKIP_IF_PLAYED:
    case CMD_ONLY_IF_PLAYED:
        return 2;
    case CMD_KILL_TTM:
    case CMD_UNK_LABEL:
        return 3;
    case CMD_ADD_TTM:
        return 4;
    default:
        if (opcode <= 0x100) {
            return 1; // CMD_SET_SCENE
        }
        std::cerr << "Unknown command found: " << opcode << std::endl;
        return 0;
    }
}

u16 SCRANTIC::ADSFile::makeHash(u16 ttm, u16 scene) {
    return ((ttm & 0xFF) << 8) | (scene & 0xFF);
}


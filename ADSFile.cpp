#include "ADSFile.h"

SCRANTIC::ADSFile::ADSFile(const std::string &name, v8 &data)
    : CompressedBaseFile(name) {

    initMnemonics();
    parseFile(data);
    parseRawScript();
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
        } if (mode == "TAGS") {
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

    v8::iterator it = rawScript.begin();

    u16 word, word2, movie, leftover;
    Command command;
    std::map<u16, std::string>::iterator tagIt;
    std::multimap<std::pair<u16, u16>, size_t> currentLabels;

    movie = 0;
    leftover = 0;

    bool first = true;

    while (it != rawScript.end()) {
        if (!leftover) {
            readUintLE(it, word);
        } else {
            word = leftover;
            leftover = 0;
        }
        command.opcode = word;
        command.data.clear();
        command.name.clear();

        switch (command.opcode) {
        case CMD_UNK_1070:
        case CMD_ADD_INIT_TTM:
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word);
            command.data.push_back(word);
            break;
        case CMD_TTM_LABEL:
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word2);
            command.data.push_back(word2);
            readUintLE(it, leftover);
            currentLabels.insert(std::make_pair(std::make_pair(word, word2), script[movie].size()));
            while (leftover == CMD_OR) {
                readUintLE(it, leftover);
                if (leftover != CMD_TTM_LABEL) {
                    std::cerr << filename << ": Error processing SKIP command! Next word not skip: "<< leftover << std::endl;
                    break;
                }
                readUintLE(it, word);
                command.data.push_back(word);
                readUintLE(it, word2);
                command.data.push_back(word2);
                currentLabels.insert(std::make_pair(std::make_pair(word, word2), script[movie].size()));
                readUintLE(it, leftover);
            }
            break;
        case CMD_SKIP_IF_LAST:
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word2);
            command.data.push_back(word2);
            readUintLE(it, leftover);
            while (leftover == CMD_OR_SKIP) {
                readUintLE(it, leftover);
                if (leftover != CMD_SKIP_IF_LAST) {
                    std::cerr << filename << ": Error processing SKIP command! Next word not skip: "<< leftover << std::endl;
                    break;
                }
                readUintLE(it, word);
                command.data.push_back(word);
                readUintLE(it, word2);
                command.data.push_back(word2);
                readUintLE(it, leftover);
            }
            break;
        case CMD_UNK_1370:
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word);
            command.data.push_back(word);
            break;
        case CMD_PLAY_MOVIE:
            break;
        case CMD_UNK_1520:
            break;
        /*u_read_le(it, word);
        command.data.push_back(word);
        u_read_le(it, word);
        command.data.push_back(word);
        u_read_le(it, word);
        command.data.push_back(word);
        u_read_le(it, word);
        command.data.push_back(word);
        u_read_le(it, word);
        command.data.push_back(word);
        break;*/
        case CMD_ADD_TTM:
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word);
            command.data.push_back(word);
            break;
        case CMD_KILL_TTM:
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word);
            command.data.push_back(word);
            readUintLE(it, word);
            command.data.push_back(word);
            break;
        case CMD_RANDOM_START:
            break;
        case CMD_UNK_3020:
            readUintLE(it, word);
            command.data.push_back(word);
            break;
        case CMD_RANDOM_END:
        case CMD_UNK_4000:
        case CMD_UNK_F010:
            break;
        case CMD_PLAY_ADS_MOVIE:
            readUintLE(it, word);
            command.data.push_back(word);
            break;
        case CMD_UNK_FFFF:
            break;
        default:
            if (command.opcode >= 0x100) {
                std::cerr << "Unkown ADS command " << command.opcode << std::endl;
            }

            tagIt = tagList.find(command.opcode);
            if (tagIt != tagList.end()) {
                command.name = tagIt->second;
            }

            if (first) {
                first = false;
            } else {
                labels.insert(std::make_pair(movie, currentLabels));
                currentLabels.clear();
            }

            movie = command.opcode;
            command.data.push_back(command.opcode);
            command.opcode = CMD_SET_SCENE;

            break;
        }

        script[movie].push_back(command);
    }

    labels.insert(std::make_pair(movie, currentLabels));
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

std::vector<SCRANTIC::Command> SCRANTIC::ADSFile::getFullMovie(u16 num) {
    std::map<u16, std::vector<Command> >::iterator it = script.find(num);
    if (it == script.end()) {
        return std::vector<Command>();
    } else {
        return it->second;
    }
}

std::multimap<std::pair<u16, u16>, size_t> SCRANTIC::ADSFile::getMovieLabels(u16 num) {
    std::multimap<u16, std::multimap<std::pair<u16, u16>, size_t> >::iterator it = labels.find(num);
    if (it == labels.end()) {
        return std::multimap<std::pair<u16, u16>, size_t>();
    }
    return it->second;
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
    mnemonics.insert({CMD_OR, "AND"});
    mnemonics.insert({CMD_PLAY_MOVIE, "PLAYMOVIE"});
    mnemonics.insert({CMD_UNK_1520, "UNK1520"});
    mnemonics.insert({CMD_RANDOM_START, "RANDSTART"});
    mnemonics.insert({CMD_RANDOM_END, "RANDEND"});
    mnemonics.insert({CMD_UNK_4000, "UNK4000"});
    mnemonics.insert({CMD_UNK_F010, "UNKF010"});
    mnemonics.insert({CMD_UNK_FFFF, "UNKFFFF"});
    //1 param
    mnemonics.insert({CMD_PLAY_ADS_MOVIE, "PLAYADS"});
    mnemonics.insert({CMD_UNK_3020, "UNK3020"});
    //2 params
    mnemonics.insert({CMD_UNK_1070, "UNK1070"});
    mnemonics.insert({CMD_ADD_INIT_TTM, "INITTTM"});
    mnemonics.insert({CMD_TTM_LABEL, "CONTINUEAFTER"});
    mnemonics.insert({CMD_SKIP_IF_LAST, "SKIPAFTER"});
    mnemonics.insert({CMD_UNK_1370, "UNK1370"});
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
    case CMD_OR:
    case CMD_PLAY_MOVIE:
    case CMD_UNK_1520:
    case CMD_RANDOM_START:
    case CMD_RANDOM_END:
    case CMD_UNK_4000:
    case CMD_UNK_F010:
    case CMD_UNK_FFFF:
        return 0;
    case CMD_SET_SCENE: // beware fake
    case CMD_PLAY_ADS_MOVIE:
    case CMD_UNK_3020:
        return 1;
    case CMD_UNK_1070:
    case CMD_ADD_INIT_TTM:
    case CMD_TTM_LABEL:
    case CMD_SKIP_IF_LAST:
    case CMD_UNK_1370:
        return 2;
    case CMD_KILL_TTM:
        return 3;
    case CMD_ADD_TTM:
        return 4;
    default:
        if (opcode <= 0x100) {
            return 1; // CMD_SET_SCENE
        }
        std::cerr << "Unkown command found: " << opcode << std::endl;
        return 0;
    }
}

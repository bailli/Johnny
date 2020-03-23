#include "ADSFile.h"

SCRANTIC::ADSFile::ADSFile(const std::string &name, v8 &data)
    : CompressedBaseFile(name) {

    parseFile(data);
    parseRawScript();
}

SCRANTIC::ADSFile::ADSFile(const std::string &filename)
    : CompressedBaseFile(filename) {

    std::ifstream in;
    in.open(filename, std::ios::binary | std::ios::in);
    in.unsetf(std::ios::skipws);

    u8 byte;
    v8 data;

    while (in.read((char*)&byte, 1)) {
        data.push_back(byte);
    }

    in.close();

    parseFile(data);
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

    /*if (!rawScript.size())
        return;*/

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

    SCRANTIC::BaseFile::writeFile(rawData, filename, "tmp/");

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

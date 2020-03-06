#include "ADSFile.h"

SCRANTIC::ADSFile::ADSFile(const std::string &name, v8 &data)
    : CompressedBaseFile(name) {

    v8::iterator it = data.begin();

    assertString(it, "VER:");

    readUintLE(it, verSize);
    version = readString(it, verSize-1);

    assertString(it, "ADS:");

    readUintLE(it, resScrTagSize);

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

    if (!rawScript.size()) {
        return;
    }

    it = rawScript.begin();

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

std::string SCRANTIC::ADSFile::getResource(u16 num)
{
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

std::multimap<std::pair<u16, u16>, size_t> SCRANTIC::ADSFile::getMovieLabels(u16 num)
{
    std::multimap<u16, std::multimap<std::pair<u16, u16>, size_t> >::iterator it = labels.find(num);
    if (it == labels.end()) {
        return std::multimap<std::pair<u16, u16>, size_t>();
    }
    return it->second;
}

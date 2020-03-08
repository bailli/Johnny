#include "TTMFile.h"

SCRANTIC::TTMFile::TTMFile(const std::string &name, v8 &data)
    : CompressedBaseFile(name) {

    v8::iterator it = data.begin();

    assertString(it, "VER:");

    readUintLE(it, verSize);
    version = readString(it, verSize-1);

    assertString(it, "PAG:");

    readUintLE(it, pagSize);
    readUintLE(it, pag);

    assertString(it, "TT3:");

    if (!handleDecompression(data, it, rawScript)) {
        return;
    }

    std::advance(it, compressedSize);

    /*if (!rawScript.size())
        return;*/

    assertString(it, "TTI:");

    readUintLE(it, fullTagSize);
    readUintLE(it, magic);

    assertString(it, "TAG:");

    readUintLE(it, tagSize);
    readUintLE(it, tagCount);

    u16 id;
    std::string desc;

    for (u16 i = 0; i < tagCount; ++i) {
        readUintLE(it, id);
        desc = readString(it);
        tagList.insert(std::pair<u16, std::string>(id, desc));
    }

    if (!rawScript.size()) {
        return;
    }

    it = rawScript.begin();

    u16 opcode;
    u16 word, scene;
    u8 length;
    Command command;
    std::map<u16, std::string>::iterator tagIt;

    scene = 0;

    while (it != rawScript.end()) {
        readUintLE(it, opcode);
        length = (opcode & 0x000F);
        command.opcode = (opcode & 0xFFF0);
        command.data.clear();
        command.name.clear();

        if ((command.opcode == CMD_SET_SCENE) || (command.opcode == CMD_SET_SCENE_LABEL)) { // && (length == 1)) // tag
            readUintLE(it, word);
            command.data.push_back(word);
            tagIt = tagList.find(word);
            if (tagIt != tagList.end()) {
                command.name = tagIt->second;
            }
        } else if (length == 0xF) { //string
            command.name = readString(it);
            if ((command.name.length() % 2) == 0) { // align to word
                readUintLE(it, length);
            }
        } else {
            for (u8 i = 0; i < length; ++i) {
                readUintLE(it, word);
                command.data.push_back(word);
            }
        }

        if (command.opcode == CMD_SET_SCENE_LABEL) {
            Command c;
            c.opcode = CMD_JMP_SCENE;
            c.data.push_back(command.data.at(0));
            script[scene].push_back(c);
        }

        if ((command.opcode == CMD_SET_SCENE) || (command.opcode == CMD_SET_SCENE_LABEL)) {
            scene = command.data.at(0);
        }

        script[scene].push_back(command);
    }

#ifdef DUMP_TTM
    std::cout << "Filename: " << filename << std::endl;
    std::string num;

    for (auto it = tagList.begin(); it != tagList.end(); ++it) {
        num = hex_to_string(it->first, std::dec);
        for (int j = num.size(); j < 3; ++j) {
            num = " " + num;
        }
        std::cout << "TAG ID " << num << ": " << it->second << std::endl;
    }

    std::cout << std::endl;

    for (auto it = script.begin(); it != script.end(); ++it) {
        std::cout << "Scene number: " << it->first << " - 0x" << hex_to_string(it->first, std::hex) << std::endl;

        for (size_t i = 0; i < it->second.size(); ++i) {
            num = hex_to_string(i, std::dec);
            for (int j = num.size(); j < 3; ++j) {
                num = " " + num;
            }
            std::cout << num << ": " << SCRANTIC::BaseFile::commandToString(it->second[i]) << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << std::endl;
#endif
}

std::vector<SCRANTIC::Command> SCRANTIC::TTMFile::getFullScene(u16 num)
{
    std::map<u16, std::vector<Command> >::iterator it = script.find(num);
    if (it == script.end()) {
        return std::vector<Command>();
    }
    return it->second;
}

std::string SCRANTIC::TTMFile::getTag(u16 num)
{
    std::map<u16, std::string>::iterator it = tagList.find(num);
    if (it == tagList.end()) {
        return std::string();
    }
    return it->second;
}


bool SCRANTIC::TTMFile::hasInit()
{
    std::map<u16, std::vector<Command> >::iterator it;
    it = script.find(0);

    if (it == script.end()) {
        return false;
    }
    return true;
}

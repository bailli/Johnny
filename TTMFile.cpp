#include "TTMFile.h"

SCRANTIC::TTMFile::TTMFile(const std::string &name, v8 &data)
    : CompressedBaseFile(name) {

    initMnemonics();

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

    parseRawScript();
}

SCRANTIC::TTMFile::TTMFile(const std::string& filename)
    : CompressedBaseFile(filename) {

    initMnemonics();

    std::ifstream in;

    in.open(filename, std::ios::in);
    if (!in.is_open()) {
        std::cerr << "TTMFile: Could not open " << filename << std::endl;
        return;
    }

    u16 i, j, k, l, m, n;

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

        if (line == "TAGS") {
            mode = line;
            continue;
        }

        if (line == "SCRIPT") {
            mode = line;
            continue;
        }

        if (mode == "TAGS") {
            std::istringstream iss(line);
            if (!(iss >> std::hex >> id)) {
                break;
            }
            tag = line.substr(line.find(" ")+1);
            tagList.insert({id, tag});
        } else if (mode == "SCRIPT") {
            mnemonic = line.substr(0, line.find(" "));
            opcode = getOpcodeFromMnemonic(mnemonic);
            count = getParamCount(opcode);

            opcode |= count;
            writeUintLE(rawScript, opcode);

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
            case 4:
                iss >> mnemonic >> std::hex >> i >> j >> k >> l;
                writeUintLE(rawScript, i);
                writeUintLE(rawScript, j);
                writeUintLE(rawScript, k);
                writeUintLE(rawScript, l);
                break;
            case 6:
                iss >> mnemonic >> std::hex >> i >> j >> k >> l >> m >> n;
                writeUintLE(rawScript, i);
                writeUintLE(rawScript, j);
                writeUintLE(rawScript, k);
                writeUintLE(rawScript, l);
                writeUintLE(rawScript, m);
                writeUintLE(rawScript, n);
                break;
            case 0xF:
                iss >> mnemonic >> tag;
                std::copy(tag.begin(), tag.end(), std::back_inserter(rawScript));
                rawScript.push_back(0);
                if (tag.size() % 2 == 0) {
                    rawScript.push_back(0);
                }
                break;
            default:
                break;
            }
        }
    }

    in.close();

    parseRawScript();
}

v8 SCRANTIC::TTMFile::repackIntoResource() {
    std::string strings[5] = { "VER:", "PAG:", "TT3:", "TTI:", "TAG:" };

    magic = 0x8000;

    compressionFlag = 1;
    v8 compressedData = RLECompress(rawScript);
    uncompressedSize = rawScript.size();
    compressedSize = compressedData.size() + 5;

    verSize = version.size() + 1;

    pagSize = 2;
    pag = countUpdateInScript();

    tagSize = 2;
    for (auto it = tagList.begin(); it != tagList.end(); ++it) {
        tagSize += 2 + it->second.size() + 1;
    }

    fullTagSize = tagSize + 8; // TAG to end
    tagCount = tagList.size();

    v8 rawData(strings[0].begin(), strings[0].end());
    BaseFile::writeUintLE(rawData, verSize);
    std::copy(version.begin(), version.end(), std::back_inserter(rawData));
    rawData.push_back(0);

    std::copy(strings[1].begin(), strings[1].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, pagSize);
    BaseFile::writeUintLE(rawData, pag);

    std::copy(strings[2].begin(), strings[2].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, compressedSize);
    BaseFile::writeUintLE(rawData, compressionFlag);
    BaseFile::writeUintLE(rawData, uncompressedSize);
    std::copy(compressedData.begin(), compressedData.end(), std::back_inserter(rawData));
    compressedSize -= 5;

    std::copy(strings[3].begin(), strings[3].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, fullTagSize);
    BaseFile::writeUintLE(rawData, magic);

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

void SCRANTIC::TTMFile::parseRawScript() {
    v8::iterator it = rawScript.begin();

    u16 opcode;
    u16 word, scene;
    u8 length;
    std::map<u16, std::string>::iterator tagIt;

    scene = 0;

    while (it != rawScript.end()) {
        readUintLE(it, opcode);
        length = (opcode & 0x000F);
        Command command;
        command.opcode = (opcode & 0xFFF0);

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
            c.data.push_back(0xFFFF);
            script[scene].push_back(c);
        }

        if ((command.opcode == CMD_SET_SCENE) || (command.opcode == CMD_SET_SCENE_LABEL)) {
            scene = command.data.at(0);
        }

        script[scene].push_back(command);
    }
}

void SCRANTIC::TTMFile::saveFile(const std::string &path) {
    std::stringstream output;

    output << "# SCRANTIC TTM file" << std::endl;
    output << "VERSION" << std::endl;
    output << version << std::endl << std::endl;

    output << "TAGS" << std::endl;
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
        length = (opcode & 0x000F);
        Command command;
        command.opcode = (opcode & 0xFFF0);
        if (command.opcode == CMD_SET_SCENE || command.opcode == CMD_SET_SCENE_LABEL) {
            output << "# new scene" << std::endl;
        }

        if (length == 0xF) { //string
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

        output << getMnemoic(command) << std::endl;
    }

    writeFile(output.str(), filename, path);
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

u16 SCRANTIC::TTMFile::countUpdateInScript() {
    u16 count = 0;
    for (auto it = script.begin(); it != script.end(); ++it) {
        for (size_t i = 0; i < it->second.size(); ++i) {
            if (it->second[i].opcode == CMD_UPDATE) {
                ++count;
            }
        }
    }
    return count;
}

void SCRANTIC::TTMFile::initMnemonics() {
    //no params
    mnemonics.insert({CMD_UNK_0080, "UNK0800"});
    mnemonics.insert({CMD_PURGE, "PURGE"});
    mnemonics.insert({CMD_UPDATE, "UPDATE"});
    //1 param
    mnemonics.insert({CMD_DELAY, "DELAY"});
    mnemonics.insert({CMD_SEL_SLOT_IMG, "IMGSLOT"});
    mnemonics.insert({CMD_SEL_SLOT_PAL, "PALSLOT"});
    mnemonics.insert({CMD_SET_SCENE_LABEL, "LABEL"});
    mnemonics.insert({CMD_SET_SCENE, "SCENE"});
    mnemonics.insert({CMD_UNK_1120, "UNK1120"});
    mnemonics.insert({CMD_JMP_SCENE, "JUMP"});
    mnemonics.insert({CMD_CLEAR_RENDERER, "CLEAR"});
    mnemonics.insert({CMD_PLAY_SOUND, "PLAYSND"});
    //string param
    mnemonics.insert({CMD_LOAD_SCREEN, "SCREEN"});
    mnemonics.insert({CMD_LOAD_BITMAP, "BITMAP"});
    mnemonics.insert({CMD_LOAD_PALETTE, "PALETTE"});
    //2 params
    mnemonics.insert({CMD_SET_COLOR, "SELCOLOR"});
    mnemonics.insert({CMD_UNK_2010, "UNK2010"});
    mnemonics.insert({CMD_UNK_2020, "TIMER"});
    mnemonics.insert({CMD_DRAW_PIXEL, "PIXEL"});
    //4 params
    mnemonics.insert({CMD_CLIP_REGION, "SETCLIP"});
    mnemonics.insert({CMD_SAVE_IMAGE, "SAVEIMG"});
    mnemonics.insert({CMD_SAVE_IMAGE_NEW, "SAVENEWIMG"});
    mnemonics.insert({CMD_UNK_A050, "UNKA050"});
    mnemonics.insert({CMD_UNK_A060, "UNKA060"});
    mnemonics.insert({CMD_DRAW_LINE, "LINE"});
    mnemonics.insert({CMD_DRAW_RECTANGLE, "RECTANGLE"});
    mnemonics.insert({CMD_DRAW_ELLIPSE, "ELLIPSE"});
    mnemonics.insert({CMD_DRAW_SPRITE, "SPRITE"});
    mnemonics.insert({CMD_DRAW_SPRITE_MIRROR, "SPRITEHINV"});
    //6 params
    mnemonics.insert({CMD_UNK_B600, "UNKB600"});
}


std::string SCRANTIC::TTMFile::getMnemoic(SCRANTIC::Command c) {
    std::string result = mnemonics[c.opcode];

    int count = getParamCount(c.opcode);

    if (!count) {
        return result;
    }

    if (count == 0xF) {
        return result + " " + c.name;
    }

    for (int i = 0; i < count; ++i) {
        std::string data = hexToString(c.data.at(i), std::hex, 4);
        result += " " + data;
    }

    return result;
}

u16 SCRANTIC::TTMFile::getOpcodeFromMnemonic(std::string &mnemonic) {
    for (auto it = mnemonics.begin(); it != mnemonics.end(); ++it) {
        if (it->second == mnemonic) {
            return it->first;
        }
    }
    return 0;
}


int SCRANTIC::TTMFile::getParamCount(u16 opcode) {
    switch (opcode) {
    case CMD_UNK_0080:
    case CMD_PURGE:
    case CMD_UPDATE:
        return 0;
    case CMD_DELAY:
    case CMD_SEL_SLOT_IMG:
    case CMD_SEL_SLOT_PAL:
    case CMD_SET_SCENE_LABEL:
    case CMD_SET_SCENE:
    case CMD_UNK_1120:
    case CMD_JMP_SCENE:
    case CMD_CLEAR_RENDERER:
    case CMD_PLAY_SOUND:
        return 1;
    case CMD_LOAD_SCREEN:
    case CMD_LOAD_BITMAP:
    case CMD_LOAD_PALETTE:
        return 0xF;
    case CMD_SET_COLOR:
    case CMD_UNK_2010:
    case CMD_UNK_2020:
    case CMD_DRAW_PIXEL:
        return 2;
    case CMD_CLIP_REGION:
    case CMD_SAVE_IMAGE:
    case CMD_SAVE_IMAGE_NEW:
    case CMD_UNK_A050:
    case CMD_UNK_A060:
    case CMD_DRAW_LINE:
    case CMD_DRAW_RECTANGLE:
    case CMD_DRAW_ELLIPSE:
    case CMD_DRAW_SPRITE:
    case CMD_DRAW_SPRITE_MIRROR:
        return 4;
    case CMD_UNK_B600:
        return 6;
    default:
        std::cerr << "Unkown command found: " << opcode << std::endl;
        return 0;
    }
}

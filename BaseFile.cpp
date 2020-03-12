#include "BaseFile.h"

#include <sstream>

SCRANTIC::BaseFile::BaseFile(const std::string &name)
    : filename(name) {

}

SCRANTIC::BaseFile::~BaseFile() {

}

std::string SCRANTIC::BaseFile::readString(std::ifstream *in, u8 length, char delimiter) {
    if (!in->is_open()) {
        return "";
    }

    std::streampos pos = in->tellg();
    std::streampos i = 0;
    std::string str = "";
    char c;

    in->read(&c, 1);
    i += 1;
    while (c != delimiter) {
        str += c;
        in->read(&c, 1);
        i += 1;
        if ((length) && (i > length)) {
            break;
        }
    }

    if (length && (str.length() != length)) {
        in->seekg(pos + static_cast<std::streampos>(length + 1), std::ios::beg);
    }

    return str;
}

std::string SCRANTIC::BaseFile::readString(v8::iterator &it, u8 length, char delimiter) {
    std::string str = "";
    u8 i = 0;

    char c = (char)*it;
    ++it;
    ++i;

    while (c != delimiter) {
        str += c;
        c = (char)*it;
        ++i;
        ++it;

        if (length && (i > length)) {
            break;
        }
    }

    if (length) {
        for ( ; i < length; ++i) {
            ++it;
        }
    }

    return str;
}

std::string SCRANTIC::BaseFile::readConstString(std::ifstream *in, u8 length) {
    if (!in->is_open()) {
        return "";
    }

    char c;
    std::string str = "";

    for (u8 i = 0; i < length; ++i) {
        in->read(&c, 1);
        str += c;
    }

    return str;
}

std::string SCRANTIC::BaseFile::readConstString(v8::iterator &it, u8 length) {
    std::string str = "";

    for (u8 i = 0; i < length; ++i) {
        str += (char)*it;
        ++it;
    }

    return str;
}

void SCRANTIC::BaseFile::assertString(v8::iterator &it, std::string expectedString) {
    std::string actualString = readConstString(it, expectedString.size());
    if (actualString != expectedString) {
        std::cerr << filename << ": \"" << expectedString << "\" expected; got \"" << actualString << "\"" << std::endl;
        throw BaseFileException();
    }
}

void SCRANTIC::BaseFile::saveFile(const std::vector<u8> &data, std::string &name, std::string path) {
    std::ofstream out;

    if (path.length() && (path[path.length()-1] != '/')) {
        path += "/";
    }

    out.open(path + name, std::ios::binary | std::ios::out);
    out.unsetf(std::ios::skipws);

    if (!out.is_open()) {
        std::cerr << "BaseFile: Could not open " << path + name << std::endl;
        return;
    }

    for (auto const &i: data) {
        out.write((char*)&i, 1);
    }

    out.close();
}

std::string SCRANTIC::BaseFile::commandToString(Command cmd, bool ads) {
    std::string ret = " ";
    std::string hex;
    size_t len = 4;

    for (size_t i = 0; i < cmd.data.size(); ++i) {
        hex = hexToString(cmd.data.at(i), std::hex);
        for (size_t j = hex.size(); j < len; ++j)
            hex = "0" + hex;
        ret += hex + " ";
    }

    if (cmd.name.length()) {
        ret += cmd.name;
    }

    if (!ads) {
        switch (cmd.opcode) {
        case CMD_UNK_0080:
            return "Unkown 0x0080" + ret;
        case CMD_PURGE:
            return "Purge" + ret;
        case CMD_UPDATE:
            return "Update" + ret;
        case CMD_DELAY:
            return "Delay" + ret;
        case CMD_SEL_SLOT_IMG:
            return "Select Image Slot" + ret;
        case CMD_SEL_SLOT_PAL:
            return "Select Palette Slot" + ret;
        case CMD_SET_SCENE_LABEL:
            return "Label Scene" + ret;
        case CMD_SET_SCENE:
            return "New Scene" + ret;
        case CMD_UNK_1120:
            return "Unkown 0x1120" + ret;
        case CMD_JMP_SCENE:
            return "Jump to Scene" + ret;
        case CMD_SET_COLOR:
            return "Set Color" + ret;
        case CMD_UNK_2010:
            return "Unkown 0x2010" + ret;
        case CMD_UNK_2020:
            return "Unkown 0x2020" + ret;
        case CMD_CLIP_REGION:
            return "Clip Region" + ret;
        case CMD_SAVE_IMAGE:
            return "Save Image" + ret;
        case CMD_SAVE_IMAGE_NEW:
            return "Save New Image" + ret;
        case CMD_DRAW_PIXEL:
            return "Draw Pixel" + ret;
        case CMD_UNK_A050:
            return "Unkown 0xA050" + ret;
        case CMD_UNK_A060:
            return "Unkown 0xA060" + ret;
        case CMD_DRAW_LINE:
            return "Draw Line" + ret;
        case CMD_DRAW_RECTANGLE:
            return "Draw Rectangle" + ret;
        case CMD_DRAW_ELLIPSE:
            return "Draw Ellipse" + ret;
        case CMD_DRAW_SPRITE:
            return "Draw Sprite (normal)" + ret;
        case CMD_DRAW_SPRITE_MIRROR:
            return "Draw Sprite (mirror)" + ret;
        case CMD_CLEAR_RENDERER:
            return "Clear Renderer" + ret;
        case CMD_UNK_B600:
            return "Unkown 0xB600" + ret;
        case CMD_PLAY_SOUND:
            return "Play Sound" + ret;
        case CMD_LOAD_SCREEN:
            return "Load Screen" + ret;
        case CMD_LOAD_BITMAP:
            return "Load Bitmap" + ret;
        case CMD_LOAD_PALETTE:
            return "Load Palette" + ret;
        default:
            return "DEFAULT 0x" + hexToString(cmd.opcode, std::hex) + ret;
        }
    } else {
        switch (cmd.opcode) {
        // ADS instructions
        case CMD_SET_SCENE:
            return "New ADS Movie" + ret;
        case CMD_UNK_1070:
            return "Unkown 0x1070" + ret;
        case CMD_ADD_INIT_TTM:
            return "Add Init TTM" + ret;
        case CMD_TTM_LABEL:
            return "Label" + ret;
        case CMD_SKIP_IF_LAST:
            return "Skip Next If Last" + ret;
        case CMD_UNK_1370:
            return "Unkown 0x1370" + ret;
        case CMD_OR_SKIP:
            return "Or (Skip)" + ret;
        case CMD_OR:
            return "Or (Cond)" + ret;
        case CMD_PLAY_MOVIE:
            return "Play Movie" + ret;
        case CMD_UNK_1520:
            return "Unkown 0x1520" + ret;
        case CMD_ADD_TTM:
            return "Add TTM" + ret;
        case CMD_KILL_TTM:
            return "Kill TTM" + ret;
        case CMD_RANDOM_START:
            return "Random Start" + ret;
        case CMD_UNK_3020:
            return "Unkown 0x3020" + ret;
        case CMD_RANDOM_END:
            return "Random End" + ret;
        case CMD_UNK_4000:
            return "Unkown 0x4000" + ret;
        case CMD_UNK_F010:
            return "Unkown 0xF010" + ret;
        case CMD_PLAY_ADS_MOVIE:
            return "Play ADS Movie" + ret;
        case CMD_UNK_FFFF:
            return "Unkown 0xFFFF" + ret;

        default:
            return "DEFAULT 0x" + hexToString(cmd.opcode, std::hex) + ret;
        }
    }
}

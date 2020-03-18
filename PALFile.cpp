#include "PALFile.h"

SCRANTIC::PALFile::PALFile(const std::string &name, v8 &data)
    : BaseFile(name) {

    v8::iterator it = data.begin();

    assertString(it, "PAL:");

    readUintLE(it, vgaSize);
    readUintLE(it, magic);

    assertString(it, "VGA:");

    readUintLE(it, palCount);
    if (palCount > 256*3) {
        std::cerr << filename << ": Palette count too large! " << palCount << std::endl;
        palCount = 256*3;
    }

    u8 r,g,b;
    SDL_Color color;
    color.a = 0;

    for (u32 i = 0; i < palCount/3; i++) {
        readUintLE(it, r);
        readUintLE(it, g);
        readUintLE(it, b);
        color.r = r*4;
        color.g = g*4;
        color.b = b*4;
        palette.push_back(color);
        color.a = 255;
    }
}

SCRANTIC::PALFile::PALFile(const std::string &filename)
    : BaseFile(filename) {

    std::ifstream in;

    in.open(filename, std::ios::in);
    if (!in.is_open()) {
        std::cerr << "BaseFile: Could not open " << filename << std::endl;
        return;
    }

    int r,g,b;
    char c1, c2;
    SDL_Colour colour;
    std::string line;
    palCount = 0;
    u8 a = 0;

    while (getline(in, line)) {
        if (line.substr(0, 1) == "#") {
            continue;
        }

        std::istringstream iss(line);
        if (!(iss >> r >> c1 >> g >> c2 >> b)) {
            break;
        }
        palette.push_back({ (u8)r, (u8)g, (u8)b, (u8)a});
        a = 255;
        palCount += 3;
    }

    in.close();
}

v8 SCRANTIC::PALFile::repackIntoResource() {
    std::string strings[2] = { "PAL:", "VGA:" };

    magic = 0x8000;
    vgaSize = palCount + 8;

    v8 rawData(strings[0].begin(), strings[0].end());
    BaseFile::writeUintLE(rawData, vgaSize);
    BaseFile::writeUintLE(rawData, magic);
    std::copy(strings[1].begin(), strings[1].end(), std::back_inserter(rawData));
    BaseFile::writeUintLE(rawData, palCount);

    u8 r,g,b;

    for (size_t i = 0; i < palette.size(); ++i) {
        r = palette[i].r/4;
        g = palette[i].g/4;
        b = palette[i].b/4;
        BaseFile::writeUintLE(rawData, r);
        BaseFile::writeUintLE(rawData, g);
        BaseFile::writeUintLE(rawData, b);
    }

    return rawData;
}

void SCRANTIC::PALFile::saveFile(const std::string &path) {
    std::stringstream output;

    output << "# SCRANTIC palette file" << std::endl
           << "# format: r,g,b" << std::endl
           << "# first entry determines transparent colour" << std::endl;

    for (size_t i = 0; i < palette.size(); ++i) {
        output << (u16)palette[i].r << ","
               << (u16)palette[i].g << ","
               << (u16)palette[i].b << std::endl;
    }

    writeFile(output.str(), filename, path);
}

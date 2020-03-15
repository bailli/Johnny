#include "RESFile.h"
#include "PALFile.h"
#include "BMPFile.h"
#include "SCRFile.h"
#include "TTMFile.h"
#include "ADSFile.h"

SCRANTIC::RESFile::RESFile(const std::string &name)
    : BaseFile(name) {

    std::ifstream in;
    in.open(filename, std::ios::binary | std::ios::in);
    in.unsetf(std::ios::skipws);

    header.reserve(6);

    u8 byte;
    for (int i = 0; i < 6; ++i) {
        readUintLE(&in, byte);
        header.push_back(byte);
    }

    resFilename = readString(&in, 12);

    if (resFilename.length() != 12) {
        std::cerr << "RESFile: Resource filename corrupt? " << resFilename << std::endl;
    }

    std::ifstream res;
    res.open(resFilename, std::ios::binary | std::ios::in);
    res.unsetf(std::ios::skipws);

    if (!res.is_open()) {
        std::cerr << "RESFile: Could not open resource file: " << resFilename << std::endl;
    }

    readUintLE(&in, resCount);

    u32 blob, offset;
    resource newRes;

    for (u16 i = 0; i < resCount; ++i) {
        newRes.data.clear();
        newRes.handle = NULL;
        readUintLE(&in, blob);
        readUintLE(&in, offset);

        res.seekg(offset, std::ios::beg);

        newRes.num = i;
        newRes.filename = readString(&res, 12);
        newRes.filetype = newRes.filename.substr(newRes.filename.rfind('.')+1);
        newRes.blob = blob;
        newRes.offset = offset;
        readUintLE(&res, newRes.size);

        for (u32 j = 0; j < newRes.size; ++j) {
            readUintLE(&res, byte);
            newRes.data.push_back(byte);
        }

        if (newRes.filetype == "PAL") {
            PALFile *pal = new PALFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(pal);
        } else if (newRes.filetype == "SCR") {
            //std::string bmpFilename = newRes.filename.substr(0, newRes.filename.rfind('.')) + ".BMP";
            //SCRFile *scr = new SCRFile(bmpFilename);
            SCRFile *scr = new SCRFile(newRes.filename, newRes.data);
            //scr->saveFile("");
            newRes.handle = static_cast<BaseFile *>(scr);
        } else if (newRes.filetype == "BMP") {
            BMPFile *bmp = new BMPFile(newRes.filename, newRes.data);
            //bmp->saveFile("");
            newRes.handle = static_cast<BaseFile *>(bmp);
        } else if (newRes.filetype == "TTM") {
            TTMFile *ttm = new TTMFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(ttm);
        } else if (newRes.filetype == "ADS") {
            ADSFile *ads = new ADSFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(ads);
            ADSFiles.push_back(newRes.filename);
        }

        resourceMap.insert(std::pair<u8, SCRANTIC::resource>(i, newRes));

        //saveFile(newRes.data, newRes.filename, "res/");
    }

    res.close();
    in.close();
}

SCRANTIC::RESFile::~RESFile() {
    for (auto i = std::begin(resourceMap); i != std::end(resourceMap); ++i) {
        if (i->second.handle != NULL) {
            delete i->second.handle;
        }
    }
}

SCRANTIC::BaseFile *SCRANTIC::RESFile::getResource(const std::string &name) {
    for (auto i = std::begin(resourceMap); i != std::end(resourceMap); ++i) {
        if (i->second.filename == name) {
            return i->second.handle;
        }
    }

    return NULL;
}

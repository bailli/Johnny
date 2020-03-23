#include "RESFile.h"
#include "PALFile.h"
#include "BMPFile.h"
#include "SCRFile.h"
#include "TTMFile.h"
#include "ADSFile.h"

#include <experimental/filesystem>

SCRANTIC::RESFile::RESFile(const std::string &name, bool readFromFile)
    : BaseFile(name) {

    std::string path = "resources/";

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

    u32 offset;
    u16 blob1, blob2;
    resource newRes;

    for (u16 i = 0; i < resCount; ++i) {
        newRes.data.clear();
        newRes.handle = NULL;
        readUintLE(&in, blob1);
        readUintLE(&in, blob2);
        readUintLE(&in, offset);

        res.seekg(offset, std::ios::beg);

        newRes.num = i;
        newRes.filename = readString(&res, 12);
        newRes.filetype = newRes.filename.substr(newRes.filename.rfind('.')+1);
        newRes.blob1 = blob1;
        newRes.blob2 = blob2;
        newRes.offset = offset;
        readUintLE(&res, newRes.size);

        for (u32 j = 0; j < newRes.size; ++j) {
            readUintLE(&res, byte);
            newRes.data.push_back(byte);
        }

        if (newRes.filetype == "PAL") {
            PALFile *resfile = readFromFile ?
                               new PALFile(path + newRes.filename) :
                               new PALFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "SCR") {
            SCRFile *resfile = readFromFile ?
                               new SCRFile(path + newRes.filename) :
                               new SCRFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "BMP") {
            BMPFile *resfile = readFromFile ?
                               new BMPFile(path + newRes.filename) :
                               new BMPFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "TTM") {
            TTMFile *resfile = readFromFile ?
                               new TTMFile(path + newRes.filename) :
                               new TTMFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "ADS") {
            ADSFile *resfile = readFromFile ?
                               new ADSFile(path + newRes.filename) :
                               new ADSFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
            ADSFiles.push_back(newRes.filename);
        }

        resourceMap.insert(std::pair<u8, SCRANTIC::resource>(i, newRes));
    }

    res.close();
    in.close();
}

void SCRANTIC::RESFile::saveNewResource(const std::string &path) {

    v8 data;
    v8 resFile;

    for (int i = 0; i < 6; ++i) {
        data.push_back(header[i]);
    }

    std::copy(resFilename.begin(), resFilename.end(), std::back_inserter(data));
    for (size_t i = resFilename.size(); i < 13; ++i) {
        data.push_back(0);
    }

    writeUintLE(data, resCount);

    u32 offset = 0;
    u32 newSize;

    for (u16 i = 0; i < resCount; ++i) {
        resource currentResource = resourceMap[i];
        writeUintLE(data, currentResource.blob1);
        writeUintLE(data, currentResource.blob2);
        writeUintLE(data, offset);

        v8 newData;
        if (currentResource.filetype == "VIN") {
            newData = currentResource.data;
         } else {
             newData = (currentResource.handle)->repackIntoResource();
         }

        std::copy(currentResource.filename.begin(), currentResource.filename.end(), std::back_inserter(resFile));
        for (size_t i = currentResource.filename.size(); i < 13; ++i) {
            resFile.push_back(0);
        }
        newSize = newData.size();
        writeUintLE(resFile, newSize);
        std::copy(newData.begin(), newData.end(), std::back_inserter(resFile));
        offset += newSize + 17;
    }

    SCRANTIC::BaseFile::writeFile(data, filename, path);
    SCRANTIC::BaseFile::writeFile(resFile, resFilename, path);
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

#include "RESFile.h"
#include "PALFile.h"
#include "BMPFile.h"
#include "SCRFile.h"
#include "TTMFile.h"
#include "ADSFile.h"
#include "VINFile.h"

#include <experimental/filesystem>
#include <algorithm>
namespace fs = std::experimental::filesystem;

SCRANTIC::RESFile::RESFile(const std::string &name, bool readFromFile)
    : BaseFile(name) {

    std::string path = "";
    size_t pathPos = name.rfind('/');
    if (pathPos != std::string::npos) {
        path = name.substr(0, pathPos+1);
    }

    if (readFromFile) {
        readFromFiles(path);
    } else {
        readFromRes(path);
    }
}

void SCRANTIC::RESFile::readFromRes(const std::string &path) {
    std::ifstream in;
    in.open(path + filename, std::ios::binary | std::ios::in);
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
    res.open(path + resFilename, std::ios::binary | std::ios::in);
    res.unsetf(std::ios::skipws);

    if (!res.is_open()) {
        std::cerr << "RESFile: Could not open resource file: " << path + resFilename << std::endl;
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
            PALFile *resfile = new PALFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "SCR") {
            SCRFile *resfile = new SCRFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "BMP") {
            BMPFile *resfile = new BMPFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "TTM") {
            TTMFile *resfile = new TTMFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "VIN") {
            VINFile *resfile = new VINFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "ADS") {
            ADSFile *resfile = new ADSFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(resfile);
        }

        resourceMap.insert(std::pair<u8, SCRANTIC::resource>(i, newRes));
    }

    res.close();
    in.close();
}

void SCRANTIC::RESFile::readFromFiles(const std::string &path) {
    std::ifstream in;

    in.open(path + filename, std::ios::in);
    if (!in.is_open()) {
        std::cerr << "RESFile: Could not open " << path << filename << std::endl;
        return;
    }

    std::string line;
    std::string resname;
    u16 blob1, blob2;
    int count = 0;

    std::string mode = "fname";

    while (getline(in, line)) {
        if (line.substr(0, 1) == "#" || line == "") {
            continue;
        }

        if (mode == "fname") {
            resFilename = line;
            mode = "header";
            continue;
        }

        if (mode == "header") {
            for (int i = 0; i < 6; ++i) {
                std::string tmp = line.substr(3*i, 2);
                header.push_back(std::stoi(tmp, 0, 16));
            }
            mode = "";
            continue;
        }

        std::istringstream iss(line);
        if (!(iss >> std::hex >> blob1 >> blob2 >> resname)) {
            break;
        }

        resource newRes;
        newRes.num = count;
        newRes.filename = resname;
        newRes.filetype = newRes.filename.substr(newRes.filename.rfind('.')+1);
        newRes.blob1 = blob1;
        newRes.blob2 = blob2;
        newRes.offset = 0;
        newRes.size = 0;

        if (newRes.filetype == "PAL") {
            PALFile *resfile = new PALFile(path + newRes.filename);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "SCR") {
            SCRFile *resfile = new SCRFile(path + newRes.filename);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "BMP") {
            BMPFile *resfile = new BMPFile(path + newRes.filename);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "TTM") {
            TTMFile *resfile = new TTMFile(path + newRes.filename);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "VIN") {
            VINFile *resfile = new VINFile(path + newRes.filename);
            newRes.handle = static_cast<BaseFile *>(resfile);
        } else if (newRes.filetype == "ADS") {
            ADSFile *resfile = new ADSFile(path + newRes.filename);
            newRes.handle = static_cast<BaseFile *>(resfile);
        }

        resourceMap.insert(std::pair<u8, SCRANTIC::resource>(count++, newRes));
    }

    resCount = count;

    in.close();
}

void SCRANTIC::RESFile::repackResources(const std::string &path, const std::string &prepackedPath) {
    fs::create_directories(path);

    std::cout << "Johnny will repack his resources :)" << std::endl;
    std::cout << "===================================" << std::endl << std::endl;

    std::string maxFiles = hexToString(resourceMap.size(), std::dec);
    int pad = maxFiles.size();
    int i = 1;

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

    for (auto it = resourceMap.begin(); it != resourceMap.end(); ++it) {
        resource currentResource = it->second;

        std::cout << "[" << hexToString(i, std::dec, pad) << "/" << maxFiles << "]: " << currentResource.filename << " ";
        writeUintLE(data, currentResource.blob1);
        writeUintLE(data, currentResource.blob2);
        writeUintLE(data, offset);

        v8 newData;
        if ((prepackedPath != "") && fs::exists(prepackedPath + currentResource.filename)) {
            newData = BaseFile::readFile(prepackedPath + currentResource.filename);
            std::cout << "prepacked file used" << std::endl;
        } else {
            newData = (currentResource.handle)->repackIntoResource();
            std::cout << "repacked" << std::endl;
        }

        std::copy(currentResource.filename.begin(), currentResource.filename.end(), std::back_inserter(resFile));
        for (size_t i = currentResource.filename.size(); i < 13; ++i) {
            resFile.push_back(0);
        }
        newSize = newData.size();
        writeUintLE(resFile, newSize);
        std::copy(newData.begin(), newData.end(), std::back_inserter(resFile));
        offset += newSize + 17;
        ++i;
    }

    std::cout << "Writing " << filename << std::endl;
    SCRANTIC::BaseFile::writeFile(data, filename, path);

    std::cout << "Writing " << resFilename << std::endl;
    SCRANTIC::BaseFile::writeFile(resFile, resFilename, path);

    std::cout << "Done." << std::endl;
}

void SCRANTIC::RESFile::unpackResources(const std::string& path, bool onlyFiles) {
    fs::create_directories(path);
    fs::create_directories(path + "RIFF/");

    std::cout << "Johnny will unpack his resources :)" << std::endl;
    std::cout << "===================================" << std::endl << std::endl;

    std::string maxFiles = hexToString(resourceMap.size(), std::dec);
    int pad = maxFiles.size();
    int i = 1;

    std::stringstream output;

    output << "# RESOURCES.MAP" << std::endl
           << "# resource filename and header followed by file list" << std::endl;

    output << resFilename << std::endl;
    for (size_t z = 0; z < header.size(); ++z) {
        output << hexToString((u16)header[z], std::hex, 2) << " ";
    }
    output << std::endl << std::endl;

    for (auto it = resourceMap.begin(); it != resourceMap.end(); ++it) {
        std::string fname = (it->second).filename;
        std::cout << "[" << hexToString(i, std::dec, pad) << "/" << maxFiles << "]: " << fname << std::endl;

        if (onlyFiles) {
            BaseFile::writeFile(it->second.data, fname, path);
        } else {
            if (it->second.filetype == "BMP") {
                fs::create_directory(path + fname);
                (*it->second.handle).saveFile(path +fname);
            } else {
                (*it->second.handle).saveFile(path);
            }
        }

        output << hexToString(it->second.blob1, std::hex, 4) << " "
               << hexToString(it->second.blob2, std::hex, 4) << " "
               << fname << std::endl;

        ++i;
    }

    if (!onlyFiles) {
        writeFile(output.str(), filename, path);
    }

    std::cout << "Done." << std::endl;
}

SCRANTIC::RESFile::~RESFile() {
    for (auto i = std::begin(resourceMap); i != std::end(resourceMap); ++i) {
        if (i->second.handle != NULL) {
            delete i->second.handle;
        }
    }
}


SDL_Color* SCRANTIC::RESFile::setPaletteForAllGraphicResources(const std::string &palFile) {
    PALFile *pal = static_cast<PALFile *>(getResource(palFile));

    for (auto it = resourceMap.begin(); it != resourceMap.end(); ++it) {
        if (it->second.filetype == "BMP") {
            static_cast<BMPFile *>(it->second.handle)->setPalette(pal->getPalette(), 256);
        } else if (it->second.filetype == "SCR") {
            static_cast<SCRFile *>(it->second.handle)->setPalette(pal->getPalette(), 256);
        }
    }

    return pal->getPalette();
}


SCRANTIC::BaseFile *SCRANTIC::RESFile::getResource(const std::string &name) {
    for (auto i = std::begin(resourceMap); i != std::end(resourceMap); ++i) {
        if (i->second.filename == name) {
            return i->second.handle;
        }
    }

    return NULL;
}

std::map<std::string, std::vector<std::string> > SCRANTIC::RESFile::getMovieList() {
    std::map<std::string, std::vector<std::string>> items;

    for (auto it = resourceMap.begin(); it != resourceMap.end(); ++it) {
        if (it->second.filetype == "ADS") {
            ADSFile * ads = static_cast<ADSFile *>(it->second.handle);
            std::string page = ads->filename;
            for (auto item : ads->tagList) {
                items[page].push_back(item.second);
            }
        }
    }

    return items;
}



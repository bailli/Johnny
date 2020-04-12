#ifndef RESFILE_H
#define RESFILE_H

#include "BaseFile.h"
#include <map>
#include "GraphicBaseFile.h"

namespace SCRANTIC {

struct resource {
    u16 num;
    std::string filename;
    u16 blob1;
    u16 blob2;
    u32 offset;
    u32 size;
    v8 data;
    std::string filetype;
    BaseFile *handle;
};

class RESFile : public BaseFile {
private:
    void readFromRes(const std::string &path);
    void readFromFiles(const std::string &path);

protected:
    v8 header;
    u16 resCount;
    std::string resFilename;
    std::map<u8, SCRANTIC::resource> resourceMap;

public:
    RESFile(const std::string &name, bool readFromFile = false);
    ~RESFile();

    SDL_Color *setPaletteForAllGraphicResources(const std::string &palFile);
    BaseFile *getResource(const std::string &name);
    std::map<std::string, std::vector<std::string>> getMovieList();

    void repackResources(const std::string &path, const std::string &prepackedPath);
    void unpackResources(const std::string &path, bool onlyFiles = false);
};

}

#endif // RESFILE_H

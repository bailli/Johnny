#ifndef RESFILE_H
#define RESFILE_H

#include "BaseFile.h"
#include <map>

namespace SCRANTIC {

struct resource
{
    u_int16_t num;
    std::string filename;
    u_int32_t blob;
    u_int32_t offset;
    u_int32_t size;
    std::vector<u_int8_t> data;
    std::string filetype;
    BaseFile *handle;
};

class RESFile : public BaseFile
{
protected:
    std::vector<u_int8_t> header;
    u_int16_t resCount;

public:
    RESFile(std::string name);
    ~RESFile();
    std::map<u_int8_t, SCRANTIC::resource> resourceMap;
    std::string resFilename;
    std::vector<std::string> ADSFiles;
    void resetTTMPositions();
    BaseFile *getResource(std::string name);
};

}

#endif // RESFILE_H

#ifndef TTMFILE_H
#define TTMFILE_H

#include "BaseFile.h"

#include <map>

namespace SCRANTIC {

class TTMFile : public BaseFile
{
protected:
    u_int32_t verSize;
    std::string version;
    u_int32_t pagSize; // always 2 ?
    u_int16_t pag;
    u_int32_t ttmSize;
    u_int8_t compressionFlag;
    u_int32_t uncompressedSize;
    u_int16_t fullTagSize;
    u_int16_t magic; // 0x8000
    u_int32_t tagSize;
    u_int16_t tagCount;
    std::map<u_int16_t, std::string> tagList;
    std::vector<u_int8_t> rawScript;
    std::map<u_int16_t, std::vector<Command> > script;
    std::map<u_int16_t, std::pair<std::map<u_int16_t, std::vector<Command> >::iterator, size_t> > scriptPositions;

public:
    TTMFile(std::string name, std::vector<u_int8_t> &data);
    Command getNextCommand(u_int16_t scene, bool newScene = false);
    void resetPositions() { scriptPositions.clear(); }
    bool hasInit();
};

}

#endif // TTMFILE_H

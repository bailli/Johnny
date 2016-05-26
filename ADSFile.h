#ifndef ADSFILE_H
#define ADSFILE_H

#include "BaseFile.h"
#include <map>
#include <tuple>

namespace SCRANTIC {

class ADSFile : public BaseFile
{
protected:
    u_int32_t verSize;
    std::string version;
    u_int32_t resScrTagSize;
    u_int32_t resSize;
    u_int16_t resCount;
    std::map<u_int16_t, std::string> resList;
    u_int32_t scrSize;
    u_int8_t compressionFlag;
    u_int32_t uncompressedSize;
    std::vector<u_int8_t> rawScript;
    u_int32_t tagSize;
    u_int16_t tagCount;
    std::map<u_int16_t, std::vector<Command> > script;
    std::map<u_int16_t, std::vector<Command> >::iterator scriptIterator;
    std::map<std::tuple<size_t, u_int16_t, u_int16_t>, size_t> labels;
    size_t scriptPos;
    int32_t currentMovie;

public:
    std::map<u_int16_t, std::string> tagList;

    ADSFile(std::string name, std::vector<u_int8_t> &data);
    Command getNextCommand(u_int16_t movie, int32_t ttm = -1, int32_t scene = -1);
    void resetScript() { currentMovie = -1; }
    std::string getResource(u_int16_t num);
    void skip();
};

}

#endif // ADSFILE_H

#ifndef ADSFILE_H
#define ADSFILE_H

#include "CompressedBaseFile.h"
#include <map>
#include <tuple>

namespace SCRANTIC {

class ADSFile : public CompressedBaseFile
{
protected:
    u32 verSize;
    std::string version;
    u32 resScrTagSize;
    u32 resSize;
    u16 resCount;
    std::map<u16, std::string> resList;
    v8 rawScript;
    u32 tagSize;
    u16 tagCount;
    std::map<u16, std::multimap<std::pair<u16, u16>, size_t> > labels;
    std::map<u16, std::vector<Command> > script;

#ifdef DUMP_ADS
    friend class Robinson;
#endif

public:
    std::map<u16, std::string> tagList;

    ADSFile(const std::string &name, v8 &data);
    std::string getResource(u16 num);
    std::vector<Command> getFullMovie(u16 num);
    std::multimap<std::pair<u16, u16>, size_t> getMovieLabels(u16 num);
};

}

#endif // ADSFILE_H

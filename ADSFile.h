#ifndef ADSFILE_H
#define ADSFILE_H

#include "CompressedBaseFile.h"
#include <map>
#include <tuple>

namespace SCRANTIC {

class ADSFile : public CompressedBaseFile
{
private:
    void parseRawScript();
    void parseRawScriptV2();
    void findLabels();
    void parseFile(v8 &data);

    std::map<u16, std::string> mnemonics;

    int getParamCount(u16 opcode);
    u16 getOpcodeFromMnemonic(std::string &mnemonic);
    void initMnemonics();
    std::string getMnemoic(Command c);

protected:
    u32 verSize;
    std::string version;
    u16 resScrTagSize;
    u16 magic;
    u32 resSize;
    u16 resCount;
    std::map<u16, std::string> resList;
    v8 rawScript;
    u32 tagSize;
    u16 tagCount;
    std::map<u16, std::multimap<std::pair<u16, u16>, size_t> > labels;
    std::map<u16, std::vector<Command> > script;

    std::map<u16, std::map<u16, std::vector<size_t>>> labelsAfter;
    std::map<u16, std::map<u16, std::vector<size_t>>> labelsTogether;

#ifdef DUMP_ADS
    friend class Robinson;
#endif

public:
    std::map<u16, std::string> tagList;

    ADSFile(const std::string &name, v8 &data);
    explicit ADSFile(const std::string &filename);
    std::string getResource(u16 num);
    std::vector<Command> getFullMovie(u16 num);
    std::multimap<std::pair<u16, u16>, size_t> getMovieLabels(u16 num);

    v8 repackIntoResource() override;
    void saveFile(const std::string &path) override;

    std::vector<Command> getBlockAfterMovie(u16 movie, u16 hash, u16 num = 0);
    std::vector<Command> getBlockTogetherWithMovie(u16 movie, u16 hash, u16 num = 0);
    std::vector<Command> getInitialBlock(u16 movie);
    size_t getLabelCountAfter(u16 movie, u16 hash);
    size_t getLabelCountTogether(u16 movie, u16 hash);

    u16 getMovieNumberFromOrder(size_t pos);
    size_t getMoviePosFromNumber(u16 number);
};

}

#endif // ADSFILE_H

#ifndef TTMFILE_H
#define TTMFILE_H

#include "CompressedBaseFile.h"

#include <map>
#include <set>

namespace SCRANTIC {

class TTMFile : public CompressedBaseFile
{
private:
    std::map<u16, std::string> mnemonics;

    int getParamCount(u16 opcode);
    u16 getOpcodeFromMnemonic(std::string mnemonic);

    u16 countUpdateInScript();

    void parseRawScript();

    void initMnemonics();
    std::string getMnemoic(Command c);

protected:
    u32 verSize;
    std::string version;
    u32 pagSize; // always 2 ?
    u16 pag;
    u16 fullTagSize;
    u16 magic; // 0x8000
    u32 tagSize;
    u16 tagCount;
    std::map<u16, std::string> tagList;
    v8 rawScript;
    std::map<u16, std::vector<Command> > script;

public:
    TTMFile(const std::string &name, v8 &data);
    TTMFile(const std::string &filename);
    std::vector<Command> getFullScene(u16 num);
    std::string getTag(u16 num);
    void saveFile(const std::string &path = "");
    bool hasInit();
    v8 repackIntoResource() override;
};

}

#endif // TTMFILE_H

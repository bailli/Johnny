#ifndef ROBINSONWALKER_H
#define ROBINSONWALKER_H

#include <string>
#include <vector>
#include <map>

#include "TTMFile.h"

namespace SCRANTIC {

class RobinsonWalker
{
private:
    TTMFile *johnWalkTTM;
    std::map<i8, std::map<i8, std::vector<SCRANTIC::Command> > > turnFrames;

    RobinsonWalker(const RobinsonWalker& other);
    TTMFile* createJohnWalkTTM(const std::string &path);
    void splitTurnAnimation();
    std::vector<SCRANTIC::Command> getTurnFrames(i8 pos, i8 start, i8 end);
    i8 getWaitTagNum(i8 pos);
    i8 getTurnTagNum(i8 pos);
    i8 getXtoYTagNum(i8 startPos, i8 endPos);

public:
    RobinsonWalker(const std::string &path, bool readFromFiles);
    std::vector<SCRANTIC::Command> getWalkScript(i8 startPos, i8 startFacing, i8 endPos, i8 endFacing);
    std::vector<std::string> getMovieList();

    ~RobinsonWalker();
};

}

#endif // ROBINSONWALKER_H

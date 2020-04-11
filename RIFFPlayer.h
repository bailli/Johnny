#ifndef RIFFPLAYER_H
#define RIFFPLAYER_H

#ifdef WIN32
#include <SDL_mixer.h>
#else
#include <SDL2/SDL_mixer.h>
#endif

#include <string>
#include "types.h"

#define MAX_AUDIO 24

namespace SCRANTIC {

class RIFFPlayer
{
private:
    std::vector<v8> readRIFFFiles(const std::string &path);

protected:
    Mix_Chunk *audioSamples[MAX_AUDIO];

public:
    explicit RIFFPlayer(const std::string &path, bool readFromFiles);
    ~RIFFPlayer();
    void play(u8 num, bool stopAllOther = true);

    static std::vector<v8> extractRIFFFiles(const std::string &filename, const std::string &path, bool writeFiles);
};

}

#endif // RIFFPLAYER_H

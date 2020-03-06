#include "RIFFPlayer.h"

#include <iostream>
#include <fstream>

#include "BaseFile.h"

SCRANTIC::RIFFPlayer::RIFFPlayer(const std::string &SCRExe)
{   
    for (u8 i = 0; i < MAX_AUDIO; ++i)
        audioSamples[i] = NULL;

    /*size_t offsets[] = { 0x1DC00, 0x20800, 0x20E00, 0x22C00, 0x24000, 0x24C00,
                         0x28A00, 0x2C600, 0x2D000, 0x2DE00, 0x32E00, 0x34400,
                         0x37200, 0x37E00, 0x39C00, 0x3AE00, 0x3E600, 0x3F400,
                         0x41200, 0x42600, 0x42C00, 0x43400, 0x45A00 };*/

    // not fully sorted yet?
    size_t offsets[] = { 0x1DC00, 0x20800, 0x20E00, 0x22C00, 0x24000, 0x24C00,
                         0x28A00, 0x2C600, 0x2D000, 0x2DE00, 0x34400, 0x32E00,
                         0x39C00, 0x43400, 0x37200, 0x37E00, 0x45A00, 0x3AE00,
                         0x3E600, 0x3F400, 0x41200, 0x42600, 0x42C00, 0x43400 };

    u32 size;
    u8 byte;
    v8 rawAudio;

    std::ifstream in;
    in.open(SCRExe, std::ios::binary | std::ios::in);
    in.unsetf(std::ios::skipws);

    if (!in.is_open()) {
        std::cerr << "RIFFPlayer: Error opening " << SCRExe << std::endl;
        return;
    }

    for (u8 i = 0; i < MAX_AUDIO; ++i) {
        in.seekg(offsets[i]+4, std::ios_base::beg);
        SCRANTIC::BaseFile::readUintLE(&in, size);
        size += 8;

        rawAudio.clear();
        rawAudio.reserve(size);

        in.seekg(offsets[i], std::ios_base::beg);

        for (u32 j = 0; j < size; ++j) {
            SCRANTIC::BaseFile::readUintLE(&in, byte);
            rawAudio.push_back(byte);
        }

        //SCRANTIC::BaseFile::saveFile(rawAudio[i], "RIFF"+std::to_string(i)+".wav");

        SDL_RWops* rwops = SDL_RWFromMem((unsigned char*)&rawAudio.front(), rawAudio.size());
        audioSamples[i] = Mix_LoadWAV_RW(rwops, 1);
    }

    in.close();
}

SCRANTIC::RIFFPlayer::~RIFFPlayer() {
    for (u8 i = 0; i < MAX_AUDIO; ++i) {
        Mix_FreeChunk(audioSamples[i]);
    }
}


void SCRANTIC::RIFFPlayer::play(u8 num, bool stopAllOther) {
    if (stopAllOther) {
        Mix_HaltChannel(-1);
    }

    Mix_PlayChannel(-1, audioSamples[num], 0);
}

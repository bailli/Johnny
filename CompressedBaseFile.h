#ifndef COMPRESSEDBASEFILE_H
#define COMPRESSEDBASEFILE_H

#include "BaseFile.h"

namespace SCRANTIC {

class CompressedBaseFile : public BaseFile {
protected:
    u32 compressedSize;
    u8 compressionFlag;
    u32 uncompressedSize;

    static v8 RLECompress(v8 const &compressedData);
    static v8 RLEDecompress(v8 const &compressedData, size_t offset = 0, u32 size = 0);
    static v8 RLE2Decompress(v8 const &compressedData, size_t offset = 0, u32 size = 0);
    static bool findInDictionary(std::vector<v8> &dictionary, v8 currentBlock, u16 &dictPos);
    static void writeBits(v8 &data, u8 &bitPos, u16 bits, u8 bitLength);
    static u16 readBits(v8 const &data, size_t &bytePos, u8 &bitPos, u16 bits);
    bool handleDecompression(v8 &data, v8::iterator &it, v8 &uncompressedData);

public:
    static v8 LZCCompress(v8 const &uncompressedData);
    static v8 LZCDecompress(v8 const &compressedData, size_t offset = 0, u32 size = 0);

    explicit CompressedBaseFile(const std::string &name) : BaseFile(name) {};
    ~CompressedBaseFile() {};
};

}

#endif // COMPRESSEDBASEFILE_H

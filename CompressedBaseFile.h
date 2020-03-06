#ifndef COMPRESSEDBASEFILE_H
#define COMPRESSEDBASEFILE_H

#include "BaseFile.h"
#include "types.h"

namespace SCRANTIC {

class CompressedBaseFile : public BaseFile {
protected:
    u32 compressedSize;
    u8 compressionFlag;
    u32 uncompressedSize;

    static v8 RLEDecompress(v8 const &compressedData, size_t offset = 0, u32 size = 0);
    static v8 RLE2Decompress(v8 const &compressedData, size_t offset = 0, u32 size = 0);
    static v8 LZWDecompress(v8 const &compressedData, size_t offset = 0, u32 size = 0);
    static u16 readBits(v8 const &data, size_t &bytePos, u8 &bitPos, u16 bits);
    bool handleDecompression(v8 &data, v8::iterator &it, v8 &uncompressedData);

public:
    explicit CompressedBaseFile(const std::string &name) : BaseFile(name) {};
    ~CompressedBaseFile() {};
};

}

#endif // COMPRESSEDBASEFILE_H

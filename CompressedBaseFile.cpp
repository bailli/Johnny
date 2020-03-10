#include "CompressedBaseFile.h"

#include <sstream>

v8 SCRANTIC::CompressedBaseFile::RLECompress(v8 const &uncompressedData) {
    v8 compressedData;
    size_t pos = 0;
    size_t differenceStart;
    u8 length = 0;
    u8 byte;

    while (pos < uncompressedData.size()) {
        byte = uncompressedData[pos++];

        while ((uncompressedData[pos] == byte)
                && (length < 0x7E) && (pos < uncompressedData.size())) {
            length++;
            pos++;
        }

        if (length) {
            compressedData.push_back((length+1) | 0x80);
            compressedData.push_back(byte);
            length = 0;
            continue;
        }

        differenceStart = pos - 1;
        while ((uncompressedData[pos] != byte)
                && (length < 0x7E) && (pos < uncompressedData.size())) {
            byte = uncompressedData[pos++];
            length++;
        }

        compressedData.push_back(length+1);
        length = 0;
        for (; differenceStart < pos; ++differenceStart) {
            compressedData.push_back(uncompressedData[differenceStart]);
        }
    }

    return compressedData;
}

v8 SCRANTIC::CompressedBaseFile::RLEDecompress(v8 const &compressedData, size_t offset, u32 size) {
    v8 decompressedData;
    u8 byte, length;

    while ((offset < compressedData.size()) && ((size == 0) || (decompressedData.size() < size))) {
        byte = compressedData[offset++];
        if ((byte & 0x80) == 0x80) {
            length = (u8)(byte & 0x7F);
            byte = compressedData[offset++];
            for (u8 i = 0; i < length; ++i) {
                decompressedData.push_back(byte);
            }
        } else {
            for (u8 i = 0; i < byte; ++i) {
                decompressedData.push_back(compressedData[offset++]);
            }
        }
    }

    return decompressedData;
}

v8 SCRANTIC::CompressedBaseFile::RLE2Decompress(v8 const &compressedData, size_t offset, u32 size) {
    v8 decompressedData;
    u8 byte, length;

    while ((offset < compressedData.size()) && ((size == 0) || (decompressedData.size() < size))) {
        byte = compressedData[offset++];
        if ((byte & 0x80) == 0x80) {
            length = compressedData[offset++];
            for (u8 i = 0; i < length; ++i) {
                decompressedData.push_back(byte & 0x7F);
            }
        } else {
            for (u8 i = 0; i < byte; ++i) {
                decompressedData.push_back(compressedData[offset++]);
            }
        }
    }

    return decompressedData;
}

void SCRANTIC::CompressedBaseFile::writeBits(v8 &data, u8 &bitPos, u16 bits, u8 bitLength) {
    bool partialByte = (bitPos != 0);
    u32 shiftedData = bits << bitPos;
    size_t byteCount = (bitLength + bitPos) / 8;
    bitPos = (bitLength + bitPos) % 8;
    if (bitPos) {
        ++byteCount;
    }

    for (size_t i = 0; i < byteCount; ++i) {
        u8 currentByte = (shiftedData >> (8*i)) & 0xFF;
        if ((i == 0) && (partialByte)) {
            data[data.size() - 1] |= currentByte;
        } else {
            data.push_back(currentByte);
        }
    }

}

u16 SCRANTIC::CompressedBaseFile::readBits(v8 const &data, size_t &bytePos, u8 &bitPos, u16 bits) {
    u16 byte = 0x00;
    for (u16 i = 0; i < bits; ++i) {
        if (bytePos >= data.size()) {
            return byte;
        }

        byte |= ((data[bytePos] >> bitPos) & 1) << i;

        if (bitPos >= 7) {
            bitPos = 0;
            ++bytePos;
        } else {
            ++bitPos;
        }
    }

    return byte;
}

bool SCRANTIC::CompressedBaseFile::FindInDictionary(std::vector<v8> &dictionary, v8 currentBlock, u16 &dictPos) {
    if (currentBlock.size() == 1) {
        dictPos = currentBlock[0];
        return true;
    }

    for (u16 i = 0; i < dictionary.size(); ++i) {
        if (currentBlock.size() != dictionary[i].size()) {
            continue;
        }

        bool mismatch = false;
        for (size_t j = 0; j < dictionary[i].size(); ++j) {
            if (dictionary[i][j] != currentBlock[j]) {
                mismatch = true;
                break;
            }
        }

        if (!mismatch) {
            dictPos = i + 257;
            return true;
        }
    }

    return false;
}

v8 SCRANTIC::CompressedBaseFile::LZCCompress(v8 const &uncompressedData) {
    v8 compressedData;
    v8 currentBlock;

    std::vector<v8> dictionary;

    u8 bitLength = 9;
    u8 bitPos = 0;
    size_t bytePos = 0;
    u32 bitCounter = 0;

    u8 nextByte;
    u16 dictPos = 0;

    while (bytePos < uncompressedData.size()) {
        nextByte = uncompressedData[bytePos++];
        currentBlock.push_back(nextByte);

        if (!FindInDictionary(dictionary, currentBlock, dictPos)) {
            if (dictionary.size() + 256 < 4095) {
                dictionary.push_back(currentBlock);
            }

            currentBlock.pop_back();
            if (currentBlock.size() == 1) {
                writeBits(compressedData, bitPos, currentBlock[0], bitLength);
            } else {
                writeBits(compressedData, bitPos, dictPos, bitLength);
            }
            bitCounter += bitLength;
            currentBlock = { nextByte };

            if (dictionary.size() + 256 >= (1 << bitLength)) {
                ++bitLength;
                bitCounter = 0;
            }

            if (dictionary.size() + 256 >= 4095) {
                writeBits(compressedData, bitPos, 256, bitLength);
                bitCounter += bitLength;
                u16 nskip = ((bitLength*8) - ((bitCounter - 1) % (bitLength*8))) - 1;
                writeBits(compressedData, bitPos, 0, nskip);
                bitCounter = 0;
                bitLength = 9;
                dictionary.clear();
            }
        }
    }

    for (size_t i = 0; i < currentBlock.size(); ++i) {
        writeBits(compressedData, bitPos, currentBlock[i], bitLength);
    }

    return compressedData;
}

v8 SCRANTIC::CompressedBaseFile::LZCDecompress(v8 const &compressedData, size_t offset, u32 size) {
    v8 decompressedData;
    std::pair<u16, u8> dictionary[4096];
    v8 decodeStack;

    u8 bitLength = 9;
    u16 freeDictPos = 257;

    u16 oldcode, newcode, code;
    size_t bytePos = offset;
    u8 bitPos = 0;
    u8 lastbyte;
    u32 bitCounter = 0;

    oldcode = readBits(compressedData, bytePos, bitPos, bitLength);
    lastbyte = oldcode;

    decompressedData.push_back((u8)oldcode);

    while ((bytePos < compressedData.size()-1) && ((size == 0) || (decompressedData.size() < size))) {
        newcode = readBits(compressedData, bytePos, bitPos, bitLength);
        bitCounter += bitLength;

        if (newcode == 256) {
            u16 nskip = ((bitLength*8) - ((bitCounter - 1) % (bitLength*8))) - 1;
            readBits(compressedData, bytePos, bitPos, nskip);
            bitLength = 9;
            freeDictPos = 256;
            bitCounter = 0;
            continue;
        }

        code = newcode;
        if (code >= freeDictPos) {
            if (decodeStack.size() >= 4096) {
                break;
            }
            decodeStack.push_back(lastbyte);
            code = oldcode;
        }

        while (code >= 256) {
            if (code > 4095) {
                break;
            }
            decodeStack.push_back(dictionary[code].second);
            code = dictionary[code].first;
        }

        decodeStack.push_back((u8)code);
        lastbyte = (u8)code;

        for (size_t i = decodeStack.size(); i > 0; --i) {
            decompressedData.push_back(decodeStack[i-1]);
        }
        decodeStack.clear();

        if (freeDictPos < 4096) {
            dictionary[freeDictPos].first = oldcode;
            dictionary[freeDictPos].second = lastbyte;
            ++freeDictPos;

            if ((bitLength < 12) && (freeDictPos >= (1 << bitLength))) {
                ++bitLength;
                bitCounter = 0;
            }
        }

        oldcode = newcode;
    }

    return decompressedData;
}
bool SCRANTIC::CompressedBaseFile::handleDecompression(v8 &data, v8::iterator &it, v8 &uncompressedData) {
    readUintLE(it, compressedSize);
    compressedSize -= 5; // substract compressionFlag and uncompressedSize
    readUintLE(it, compressionFlag);
    readUintLE(it, uncompressedSize);

    size_t i = std::distance(data.begin(), it);

    switch (compressionFlag) {
    case 0x00:
        uncompressedData = v8(it, (it+uncompressedSize));
        break;
    case 0x01:
        uncompressedData = RLEDecompress(data, i, uncompressedSize);
//         {   
//             v8 recompressedData = RLECompress(uncompressedData);
//             v8 derecompressedData = RLEDecompress(recompressedData, 0, uncompressedSize);
//             std::cout << filename << ": compression size " << (u32)compressedSize << std::endl;
//             std::cout << filename << ": recompression size " << recompressedData.size() << std::endl;
//             for (u32 j = 0; j < uncompressedSize; ++j) {
//                 if (derecompressedData[j] != uncompressedData[j]) {
//                     std::cout << filename << ": >>>>>>>>>>>>>>>>>>>>>>>>>> Recompression did not work" << std::endl;
//                     break;
//                 }
//             }
//         }
        break;
    case 0x02:
        uncompressedData = LZCDecompress(data, i, uncompressedSize);
//         {
//             std::cout << filename << ": Compression flag: " << (i16)compressionFlag << std::endl;
//             v8 recompressedData = LZCCompress(uncompressedData);
//             v8 derecompressedData = LZCDecompress(recompressedData, 0, uncompressedSize);
//             //std::cout << filename << ": compression size " << (u32)compressedSize << std::endl;
//             //std::cout << filename << ": recompression size " << recompressedData.size() << std::endl;
//             bool mismatch = false;
//             for (u32 j = 0; j < uncompressedSize; ++j) {
//                 if (derecompressedData[j] != uncompressedData[j]) {
//                     std::cout << filename << ": >>>>>>>>>>>>>>>>>>>>>>>>>> Recompression did not work" << std::endl;
//                     mismatch = true;
//                     break;
//                 }
//             }
//             if (!mismatch) {
//                 std::cout << filename << ": compression size difference " << (i32)(recompressedData.size() - compressedSize) << std::endl;
//             }
//         }

        break;
    case 0x03:
        uncompressedData = RLE2Decompress(data, i, uncompressedSize);
        break;
    default:
        std::cerr << filename << ": unhandled compression type: "
                  << (i16)compressionFlag << std::endl;
    }

    if (uncompressedSize != (u32)uncompressedData.size()) {
        std::cerr << filename << ": decompression error: expected size: "
                  << (size_t)uncompressedSize  << " - got " << uncompressedData.size()
                  << " type " << (i16)compressionFlag << std::endl;
        return false;
    }

    if (!uncompressedData.size()) {
        return false;
    }
    return true;
}

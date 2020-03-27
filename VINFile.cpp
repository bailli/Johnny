#include "VINFile.h"

SCRANTIC::VINFile::VINFile(const std::string &name, v8 &data)
    : BaseFile(name),
    rawData(data.begin(), data.end()) {
}

SCRANTIC::VINFile::VINFile(const std::string &filename)
    : BaseFile(filename),
    rawData(BaseFile::readFile(filename)) {

}

v8 SCRANTIC::VINFile::repackIntoResource() {
    return rawData;
}

void SCRANTIC::VINFile::saveFile(const std::string &path) {
    writeFile(rawData, filename, path);
}

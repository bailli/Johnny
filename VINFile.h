#ifndef VINFILE_H
#define VINFILE_H

#include "BaseFile.h"

namespace SCRANTIC {

class VINFile : public BaseFile
{
protected:
    v8 rawData;

public:
    VINFile(const std::string &name, v8 &data);
    explicit VINFile(const std::string &filename);
    v8 repackIntoResource() override;
    void saveFile(const std::string &path);
};

}

#endif // VINFILE_H

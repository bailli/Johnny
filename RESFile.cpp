#include "RESFile.h"
#include "PALFile.h"
#include "BMPFile.h"
#include "SCRFile.h"
#include "TTMFile.h"
#include "ADSFile.h"

SCRANTIC::RESFile::RESFile(std::string name) : BaseFile(name)
{
    std::ifstream in;
    in.open(filename, std::ios::binary | std::ios::in);
    in.unsetf(std::ios::skipws);

    header.reserve(6);

    u_int8_t byte;
    for (int i = 0; i < 6; ++i)
    {
        u_read_le(&in, byte);
        header.push_back(byte);
    }

    resFilename = read_string(&in, 12);

    if (resFilename.length() != 12)
        std::cerr << "RESFile: Resource filename corrupt? " << resFilename << std::endl;

    std::ifstream res;
    res.open(resFilename, std::ios::binary | std::ios::in);
    res.unsetf(std::ios::skipws);

    if (!res.is_open())
        std::cerr << "RESFile: Could not open resource file: " << resFilename << std::endl;

    u_read_le(&in, resCount);

    u_int32_t blob, offset;
    resource newRes;

    for (int i = 0; i < resCount; ++i)
    {
        newRes.data.clear();
        newRes.handle = NULL;
        u_read_le(&in, blob);
        u_read_le(&in, offset);

        res.seekg(offset, std::ios::beg);

        newRes.num = i;
        newRes.filename = read_string(&res, 12);
        newRes.filetype = newRes.filename.substr(newRes.filename.rfind('.')+1);
        newRes.blob = blob;
        newRes.offset = offset;
        u_read_le(&res, newRes.size);

        for (u_int32_t j = 0; j < newRes.size; ++j)
        {
            u_read_le(&res, byte);
            newRes.data.push_back(byte);
        }

        if (newRes.filetype == "PAL")
        {
            PALFile *pal = new PALFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(pal);
        }
        else if (newRes.filetype == "SCR")
        {
            SCRFile *scr = new SCRFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(scr);
        }
        else if (newRes.filetype == "BMP")
        {
            BMPFile *bmp = new BMPFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(bmp);
        }
        else if (newRes.filetype == "TTM")
        {
            TTMFile *ttm = new TTMFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(ttm);
        }
        else if (newRes.filetype == "ADS")
        {
            ADSFile *ads = new ADSFile(newRes.filename, newRes.data);
            newRes.handle = static_cast<BaseFile *>(ads);
            ADSFiles.push_back(newRes.filename);
        }

        resourceMap.insert(std::pair<u_int8_t, SCRANTIC::resource>(i, newRes));

        //saveFile(newRes.data, newRes.filename, "res/");
    }

    res.close();
    in.close();
}

SCRANTIC::RESFile::~RESFile()
{
    for(auto i = std::begin(resourceMap); i != std::end(resourceMap); ++i)
    {
        if (i->second.handle != NULL)
            delete i->second.handle;
    }
}

SCRANTIC::BaseFile *SCRANTIC::RESFile::getResource(std::string name)
{
    for(auto i = std::begin(resourceMap); i != std::end(resourceMap); ++i)
        if (i->second.filename == name)
            return i->second.handle;

    return NULL;
}

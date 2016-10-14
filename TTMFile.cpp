#include "TTMFile.h"

SCRANTIC::TTMFile::TTMFile(std::string name, std::vector<u_int8_t> &data)
    : BaseFile(name)
{
    std::vector<u_int8_t>::iterator it = data.begin();

    std::string tmp = read_const_string(it, 4);
    if (tmp != "VER:")
    {
        std::cerr << filename << ": \"VER:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, verSize);
    read_string(it, verSize-1);

    tmp = read_const_string(it, 4);
    if (tmp != "PAG:")
    {
        std::cerr << filename << ": \"PAG:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, pagSize);
    u_read_le(it, pag);

    tmp = read_const_string(it, 4);
    if (tmp != "TT3:")
    {
        std::cerr << filename << ": \"TT3:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, ttmSize);
    ttmSize -= 5; // substract compressionFlag and uncompressedSize
    u_read_le(it, compressionFlag);
    u_read_le(it, uncompressedSize);

    size_t i = std::distance(data.begin(), it);

    switch (compressionFlag)
    {
    case 0x00: rawScript = std::vector<u_int8_t>(it, (it+ttmSize)); break;
    case 0x01: rawScript = RLEDecompress(data, i, uncompressedSize); break;
    case 0x02: rawScript = LZWDecompress(data, i, uncompressedSize); break;
    case 0x03: rawScript = RLE2Decompress(data, i, uncompressedSize); break;
    default: std::cerr << filename << ": unhandled compression type: " << (int16_t)compressionFlag << std::endl;
    }

    if (uncompressedSize != (u_int32_t)rawScript.size())
        std::cerr << filename << ": decompression error: expected size: " << (size_t)uncompressedSize  << " - got " << rawScript.size() << std::endl;

    std::advance(it, ttmSize);

    /*if (!rawScript.size())
        return;*/

    tmp = read_const_string(it, 4);
    if (tmp != "TTI:")
    {
        std::cerr << filename << ": \"TTI:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, fullTagSize);
    u_read_le(it, magic);

    tmp = read_const_string(it, 4);
    if (tmp != "TAG:")
    {
        std::cerr << filename << ": \"TAG:\" expected; got" << tmp << std::endl;
        return;
    }

    u_read_le(it, tagSize);
    u_read_le(it, tagCount);

    u_int16_t id;
    std::string desc;

    for (u_int16_t i = 0; i < tagCount; ++i)
    {
        u_read_le(it, id);
        desc = read_string(it);
        tagList.insert(std::pair<u_int16_t, std::string>(id, desc));
    }

    if (!rawScript.size())
        return;

    it = rawScript.begin();

    u_int16_t opcode;
    u_int16_t word, scene;
    u_int8_t length;
    Command command;
    std::map<u_int16_t, std::string>::iterator tagIt;

    scene = 0;

    while (it != rawScript.end())
    {
        u_read_le(it, opcode);
        length = (opcode & 0x000F);
        command.opcode = (opcode & 0xFFF0);
        command.data.clear();
        command.name.clear();

        if ((command.opcode == CMD_SET_SCENE) || (command.opcode == CMD_SET_SCENE_LABEL))// && (length == 1)) // tag
        {
            u_read_le(it, word);
            command.data.push_back(word);
            tagIt = tagList.find(word);
            if (tagIt != tagList.end())
                command.name = tagIt->second;
        }
        else if (length == 0xF) //string
        {
            command.name = read_string(it);
            if ((command.name.length() % 2) == 0)  // align to word
                u_read_le(it, length);
        }
        else
        {
            for (u_int8_t i = 0; i < length; ++i)
            {
                u_read_le(it, word);
                command.data.push_back(word);
            }
        }

        if (command.opcode == CMD_SET_SCENE_LABEL)
        {
            Command c;
            c.opcode = CMD_JMP_SCENE;
            c.data.push_back(command.data.at(0));
            script[scene].push_back(c);
        }

        if ((command.opcode == CMD_SET_SCENE) || (command.opcode == CMD_SET_SCENE_LABEL))
            scene = command.data.at(0);

        script[scene].push_back(command);
    }
}

/*SCRANTIC::Command SCRANTIC::TTMFile::getNextCommand(u_int16_t scene, bool newScene)
{
    std::map<u_int16_t, std::vector<Command> >::iterator it;
    it = script.find(scene);

    Command cmd;

    if (it  == script.end())
    {
        cmd.opcode = CMD_INTER_NOTFOUND;
        return cmd;
    }

    if (!newScene && (scene == currentScene))
    {
        ++scriptPos;
        if (scriptPos >= scriptIterator->second.size())
        {
            Command cmd;
            cmd.opcode = CMD_INTER_END;
            return cmd;
        }

        return scriptIterator->second[scriptPos];
    }
    else
    {
        scriptPos = 0;
        scriptIterator = it;
        currentScene = scene;
        return scriptIterator->second[scriptPos];
    }
}
*/
std::vector<SCRANTIC::Command> SCRANTIC::TTMFile::getFullScene(u_int16_t num)
{
    std::map<u_int16_t, std::vector<Command> >::iterator it = script.find(num);
    if (it == script.end())
        return std::vector<Command>();
    else
        return it->second;
}

std::string SCRANTIC::TTMFile::getTag(u_int16_t num)
{
    std::map<u_int16_t, std::string>::iterator it = tagList.find(num);
    if (it == tagList.end())
        return std::string();
    else
        return it->second;
}


bool SCRANTIC::TTMFile::hasInit()
{
    std::map<u_int16_t, std::vector<Command> >::iterator it;
    it = script.find(0);

    if (it == script.end())
        return false;
    else
        return true;
}

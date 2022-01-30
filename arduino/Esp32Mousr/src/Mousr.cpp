#include "Mousr.h"
#include <cstring>
#include <vector>
std::vector<uint8_t> raw;

void initialize(std::string data)
{
    for (unsigned i = 0; i < data.length(); i += 2)
    {
        std::string ss = data.substr(i, 2);

        if (i == 0 &&
            (ss == "0x" || ss == "0X"))
        {
            continue;
        }

        uint8_t b = strtol(ss.c_str(), nullptr, 16);
        raw.push_back(b);
    }
}

MousrData::MousrData(const MousrMessage msg, const MousrCommand cmd, std::vector<uint8_t> data)
{
    raw.push_back((uint8_t)msg);
    int length = data.size();
    if (length < 12)
    {
        length = 12;
    }

    for (int i = 0; i < length; i++)
    {
        raw.push_back(data[i]);
    }

    raw.push_back((uint8_t)cmd);
    raw.push_back(0);
}

MousrData::MousrData(const uint8_t *data, size_t length)
{
    for (int i = 0; i < length; i++)
    {
        raw.push_back(data[i]);
    }
}

MousrData::MousrData(const char *data)
{
    initialize(std::string(data));
}

MousrData::MousrData(std::string data)
{
    initialize(data);
}

MousrData::~MousrData()
{
    raw.resize(0);
    //s_writeLogLn("MousrData: deallocating");
}

size_t MousrData::getMessageLength()
{
    return raw.size();
}

MousrMessage MousrData::getMessageKind()
{
    return (MousrMessage)raw.front();
}

std::string MousrData::toString()
{
    //s_writeLogF("MousrData::toString: %p (%zu)\n", raw.data(), raw.size());
    return MousrData::toString(raw.data(), raw.size());
}

void MousrData::getRawMessageData(uint8_t **data, size_t &length)
{
    length = raw.size();
    *data = (uint8_t *)malloc(length);
    memcpy(*data, raw.data(), length);
}

MousrMessageData *MousrData::getMessageData()
{
    return (MousrMessageData *)raw.data();
}
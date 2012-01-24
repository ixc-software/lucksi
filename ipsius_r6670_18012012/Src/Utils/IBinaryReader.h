#ifndef IBINARYREADER_H
#define IBINARYREADER_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace Utils
{    
    class IBinaryReader : public IBasicInterface
    {
    public:
        virtual Platform::byte ReadByte() = 0; // can throw
        virtual bool ReadBool() = 0; // can throw
        virtual char ReadChar() = 0; // can throw
        virtual Platform::word ReadWord() = 0; // can throw
        virtual Platform::int32 ReadInt32() = 0; // can throw
        virtual Platform::int64 ReadInt64() = 0; // can throw
        virtual Platform::dword ReadDword() = 0; // can throw
        virtual float ReadFloat() = 0; // can throw
        virtual double ReadDouble() = 0; // can throw
        virtual void ReadString(std::string &data, size_t size) = 0; // can throw
        virtual void ReadWstring(std::wstring &data, size_t size) = 0; // can throw
        virtual void ReadVectorByte(std::vector<Platform::byte> &data, size_t size) = 0; // can throw      
        virtual void ReadData(void *pDest, size_t size) = 0; // can throw      
    };
} // namespace Utils

#endif

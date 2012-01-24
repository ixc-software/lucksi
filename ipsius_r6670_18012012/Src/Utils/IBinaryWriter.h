#ifndef IBINARYWRITER_H
#define IBINARYWRITER_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace Utils
{
    using Platform::byte;
    using Platform::word;
    using Platform::dword;
    using Platform::int32;
    using Platform::int64;

     class IBinaryWriter : public IBasicInterface
     {
     public:
         virtual IBinaryWriter& WriteByte(byte data) = 0; // can throw        
         virtual IBinaryWriter& WriteBool(bool data) = 0; // can throw      
         virtual IBinaryWriter& WriteChar(char data) = 0; // can throw        
         virtual IBinaryWriter& WriteWord(word data) = 0; // can throw         
         virtual IBinaryWriter& WriteDword(dword data) = 0; // can throw       
         virtual IBinaryWriter& WriteInt32(int32 data) = 0; // can throw         
         virtual IBinaryWriter& WriteInt64(int64 data) = 0; // can throw        
         virtual IBinaryWriter& WriteFloat(float data) = 0; // can throw        
         virtual IBinaryWriter& WriteDouble(double data) = 0; // can throw      
         virtual IBinaryWriter& WriteString(const std::string &data) = 0; // can throw      
         virtual IBinaryWriter& WriteWstring(const std::wstring &data) = 0; // can throw       
         virtual IBinaryWriter& WriteVectorByte(const std::vector<byte> &data) = 0; // can throw        
         virtual IBinaryWriter& WriteData(const void *pData, size_t size) = 0; // can throw
     };
} // namespace Utils

#endif

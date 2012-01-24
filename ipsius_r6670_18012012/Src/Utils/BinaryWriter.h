
#ifndef __BINARYWRITER__
#define __BINARYWRITER__

// BinaryWriter.h

#include "IBiRWStream.h"
#include "IBinaryWriter.h"

namespace Utils
{
    using Platform::byte;
    using Platform::word;
    using Platform::dword;
    using Platform::int32;
    using Platform::int64;

    // Write data to stream TStream.
    // TStream has to implement Utils::IBiWriterStream interface
    // or has the same semantic
    template<class TStream>
    class BinaryWriter :
        public boost::noncopyable,
        public IBinaryWriter
    {
        TStream &m_stream;
        
        /*
            Byte order:
             - on platform: little-endian;
             - in stream: little-endian.
        */
    
        
    public:
        BinaryWriter(TStream &stream) : m_stream(stream)
        {
        }

        
        IBinaryWriter& WriteByte(byte data) // can throw
        {
            m_stream.Write(data);
            return *this;
        }
        

        IBinaryWriter& WriteBool(bool data) // can throw
        {
            byte b = (data == true)? 1 : 0;
            m_stream.Write(b);
            return *this;
        }

        IBinaryWriter& WriteChar(char data) // can throw
        {
            return WriteByte(static_cast<byte>(data));
        }

        IBinaryWriter& WriteWord(word data) // can throw
        {
            return WriteData(&data, sizeof(data));
        }

        IBinaryWriter& WriteDword(dword data) // can throw
        {
            return WriteData(&data, sizeof(data));
        }

        IBinaryWriter& WriteInt32(int32 data) // can throw
        {
            return WriteData(&data, sizeof(data));
        }

        IBinaryWriter& WriteInt64(int64 data) // can throw
        {
            return WriteData(&data, sizeof(data));
        }

        IBinaryWriter& WriteFloat(float data) // can throw
        {
            return WriteData(&data, sizeof(data));
        }

        IBinaryWriter& WriteDouble(double data) // can throw
        {
            return WriteData(&data, sizeof(data));
        }

        IBinaryWriter& WriteString(const std::string &data) // can throw
        {
            return (data.size() == 0) ? (*this) : WriteData(&data.at(0), data.size());
        }

        IBinaryWriter& WriteWstring(const std::wstring &data) // can throw
        {
            if (data.size() == 0) return *this;
            
            size_t sizeInBytes = data.size() * sizeof(wchar_t);
            return WriteData(&data.at(0), sizeInBytes);
        }

        IBinaryWriter& WriteVectorByte(const std::vector<byte> &data)
        {
            return (data.size() == 0) ? (*this) : WriteData(&data.at(0), data.size());
        }

        IBinaryWriter& WriteData(const void *pData, size_t size) // can throw
        {
            if (pData == 0) ESS_ASSERT(size == 0);
            else m_stream.Write(pData, size);

            return *this;
        }
    };

    // -----------------------------------------------------------------

    // result must be deleted by caller
    template<class TStream>
    IBinaryWriter* CreateBinaryWriter(TStream &stream)
    {
        return new BinaryWriter<TStream>(stream);
    }

     
} // namespace Utils

#endif

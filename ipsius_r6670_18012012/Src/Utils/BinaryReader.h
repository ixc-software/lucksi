
#ifndef __BINARYREADER__
#define __BINARYREADER__

#include "IBiRWStream.h"
#include "IBinaryReader.h"


namespace Utils
{
    using Platform::byte;
    using Platform::word;
    using Platform::dword;
    using Platform::int32;
    using Platform::int64;

    // Read data from stream TStream.
    // TStream has to implement Utils::IBiReaderStream interface
    // or has the same semantic
    template<class TStream>
    class BinaryReader :         
        public boost::noncopyable,
        public IBinaryReader
    {
        TStream &m_stream;
        
        /*
            Byte order:
             - on platform: little-endian;
             - in stream: little-endian.
        */
        
    // IBinaryReader impl:
    public:
        BinaryReader(TStream &stream) : m_stream(stream)
        {
        }

        byte ReadByte() // can throw
        {
            return m_stream.ReadByte();
        }

        bool ReadBool() // can throw
        {
            byte res = m_stream.ReadByte();
            return (res == 0)? false : true;
        }

        char ReadChar() // can throw
        {
            return static_cast<char>(ReadByte());
        }
        
        word ReadWord() // can throw
        {
            word res = 0;
            ReadData(&res, sizeof(res));
            return res;
        }

        int32 ReadInt32() // can throw
        {
            int32 res = 0;
            ReadData(&res, sizeof(res));
            return res;
        }

        int64 ReadInt64() // can throw
        {
            int64 res = 0;
            ReadData(&res, sizeof(res));
            return res;
        }
        
        dword ReadDword() // can throw
        {
            dword res = 0;
            ReadData(&res, sizeof(res));
            return res;
        }

        float ReadFloat() // can throw
        {
            float res = 0;
            ReadData(&res, sizeof(res));
            return res;
        }

        double ReadDouble() // can throw
        {
            double res = 0;
            ReadData(&res, sizeof(res));
            return res;
        }

        void ReadString(std::string &data, size_t size) // can throw
        {
            ESS_ASSERT(size != 0);
            data.resize(size);
            ReadData(&data.at(0), size);
        }

        void ReadWstring(std::wstring &data, size_t size) // can throw
        {
            ESS_ASSERT(size != 0);
            data.resize(size);
            size_t sizeInBytes = size * sizeof(wchar_t);
            ReadData(&data.at(0), sizeInBytes);
        }

        void ReadVectorByte(std::vector<byte> &data, size_t size) // can throw
        {
            ESS_ASSERT(size != 0);
            data.resize(size);
            ReadData(&data.at(0), size);
        }
        
        void ReadData(void *pDest, size_t size) // can throw
        {
            // ESS_ASSERT(size != 0);
            m_stream.Read(pDest, size);
        }
    };

    // ----------------------------------------

    // BinaryReader with stream ownership
    template<class TStream>
    class BinaryReaderEx : public BinaryReader<TStream>
    {
        boost::scoped_ptr<TStream> m_stream;

    public:

        BinaryReaderEx(TStream *pStream) : BinaryReader<TStream>(*pStream)
        {
            ESS_ASSERT(pStream != 0);

            m_stream.reset(pStream);
        }
        
    };

    // ----------------------------------------

    // result must be deleted by caller
    template<class TStream>
    IBinaryReader* CreateBinaryReader(TStream &stream)
    {
        return new BinaryReader<TStream>(stream);
    }

    IBinaryReader* CreateBinaryReader(const void *pData, int size);
    IBinaryReader* CreateBinaryReader(const std::vector<Platform::byte> &data);
    
} // namespace Utils 

#endif

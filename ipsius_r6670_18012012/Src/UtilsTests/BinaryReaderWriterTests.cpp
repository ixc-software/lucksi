
#include "stdafx.h"
#include "BinaryReaderWriterTests.h"

#include "Utils/IBiRWStream.h"
#include "Utils/BinaryReader.h"
#include "Utils/BinaryWriter.h"

#include "Utils/MemReaderStream.h"
#include "Utils/MemWriterStream.h"


namespace 
{
    using namespace Utils; 
    typedef std::vector<byte> ByteVector;

    // --------------------------------------------------
    
    const size_t CBufferSize = 100;

    // --------------------------------------------------

    // const to write in buffer
    const byte CByte = 15;
    const bool CBool = true;
    const char CChar = 'j';
    const word CWord = 700;
    const dword CDword = 789456;
    const int32 CInt32 = -145896;
    const int64 CInt64 = -12345678;
    const float CFloat = 156.0;
    const double CDouble = 156.1234589;
    const std::string CString = "abcdef";
    const std::wstring CWstring = L"asdf";
    const byte CDataBlockSize = 4;
    const byte CDataBlock[CDataBlockSize] = {1, 2, 3, 4};
    const std::vector<byte> CVectorByte = std::vector<byte>(CDataBlockSize, CByte);

    // --------------------------------------------------
    
    // Reader and Writer streams must point to the same buffer
    class BRWTests
    {
        ByteVector m_buff;
        MemWriterStream  m_wStream; // implements Utils::IBiWriterStream
        MemReaderStream m_rStream;  // has the same semantic as Utils::IBiReaderStream
        BinaryReader<MemReaderStream> m_reader;
        BinaryWriter<MemWriterStream> m_writer;
        

        void WriteData()
        {
            m_writer.WriteByte(CByte);
            m_writer.WriteBool(CBool);
            m_writer.WriteChar(CChar);
            m_writer.WriteWord(CWord);
            m_writer.WriteDword(CDword);
            m_writer.WriteInt32(CInt32);
            m_writer.WriteInt64(CInt64);
            m_writer.WriteFloat(CFloat);
            m_writer.WriteDouble(CDouble);
            m_writer.WriteString(CString);
            m_writer.WriteWstring(CWstring);
            m_writer.WriteVectorByte(CVectorByte);
            m_writer.WriteData(CDataBlock, CDataBlockSize);

            // write data with 0 size
            m_writer.WriteString("");
            m_writer.WriteWstring(L"");
            m_writer.WriteVectorByte(std::vector<byte>());
            // error only if size == 0 && pData != 0
            m_writer.WriteData(CDataBlock, 0);
            m_writer.WriteData(0, 0);
        }

        void ReadAndCheck()
        {
            TUT_ASSERT(m_reader.ReadByte() == CByte);
            TUT_ASSERT(m_reader.ReadBool() == CBool);
            TUT_ASSERT(m_reader.ReadChar() == CChar);
            TUT_ASSERT(m_reader.ReadWord() == CWord);
            TUT_ASSERT(m_reader.ReadDword() == CDword);
            TUT_ASSERT(m_reader.ReadInt32() == CInt32);
            TUT_ASSERT(m_reader.ReadInt64() == CInt64);
            TUT_ASSERT(m_reader.ReadFloat() == CFloat); 
            TUT_ASSERT(m_reader.ReadDouble() == CDouble);

            std::string readStr;
            m_reader.ReadString(readStr, CString.size());
            TUT_ASSERT(readStr == CString);
            
            std::wstring readWstr;
            m_reader.ReadWstring(readWstr, CWstring.size());
            TUT_ASSERT(readWstr == CWstring);

            std::vector<byte> readVector;
            m_reader.ReadVectorByte(readVector, CDataBlockSize);
                
            byte readData[CDataBlockSize];
            m_reader.ReadData(readData, CDataBlockSize);

            for (int i = 0; i < CDataBlockSize; ++i)
            {
                TUT_ASSERT(readVector.at(i) == CVectorByte.at(i));
                TUT_ASSERT(readData[i] == CDataBlock[i]);
            }
        }
        
    public:
        BRWTests(): 
            m_buff(CBufferSize, 1), 
            m_wStream(&*m_buff.begin(), CBufferSize), 
            m_rStream(&*m_buff.begin(), CBufferSize), 
            m_reader(m_rStream),
            m_writer(m_wStream) 
        {
            WriteData();
            ReadAndCheck();

            // exceptions're checked in MemReader/WriterStream tests
        }
    };
    
    
} // namespace

// -------------------------------------------

namespace UtilsTests
{
    void BinaryReaderWriterTests(bool silentMode)
    {
        BRWTests tests;

        if (!silentMode) std::cout << "BinaryReader/Writer tests okay." << std::endl;
    }
    
} // namespace UtilsTests

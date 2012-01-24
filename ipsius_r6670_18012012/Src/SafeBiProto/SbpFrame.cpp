
#include "stdafx.h"
#include "SbpFrame.h"

#include "Utils/MemReaderStream.h"
#include "Utils/BinaryReader.h"
#include "SbpGetTypeInfo.h"
#include "SbpFrameHeader.h"


namespace SBProto
{
    Frame::Frame(TypeInfoDesc type, const void *pData, dword length) : 
        m_type(TypeInfoTable().Find(type)), m_pData(pData), m_length(length)
    {
        /*
        ESS_ASSERT(m_length != 0);
        ESS_ASSERT(m_pData != 0);
        */
        if (pData == 0) ESS_ASSERT(length == 0);
        
    }

    // ---------------------------------------------------------------
    
    byte Frame::AsByte() const
    {
        if (m_type.TypeID != TypeByte) ESS_THROW(BadFieldType);

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);
        
        return reader.ReadByte();
    }

    // ---------------------------------------------------------------

    bool Frame::AsBool() const
    {
        if (m_type.TypeID != TypeBool) ESS_THROW(BadFieldType);;

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        return reader.ReadBool();
    }

    // ---------------------------------------------------------------

    /*
    char Frame::AsChar() const
    {
        ESS_ASSERT(m_type.TypeID == TypeChar);

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        return reader.ReadChar();
    }
    */

    // ---------------------------------------------------------------

    word Frame::AsWord() const
    {
        if (m_type.TypeID != TypeWord) ESS_THROW(BadFieldType);

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        return reader.ReadWord();
    }

    // ---------------------------------------------------------------

    int32 Frame::AsInt32() const
    {
        if (m_type.TypeID != TypeInt32) ESS_THROW(BadFieldType);

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        return reader.ReadInt32();
    }

    // ---------------------------------------------------------------

    int64 Frame::AsInt64() const
    {
        if (m_type.TypeID != TypeInt64) ESS_THROW(BadFieldType);

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        return reader.ReadInt64();
    }

    // ---------------------------------------------------------------

    dword Frame::AsDword() const
    {
        if (m_type.TypeID != TypeDword) ESS_THROW(BadFieldType);

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        return reader.ReadDword();
    }

    // ---------------------------------------------------------------

    float Frame::AsFloat() const
    {
        if (m_type.TypeID != TypeFloat) ESS_THROW(BadFieldType);

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        return reader.ReadFloat();
    }

    // ---------------------------------------------------------------

    double Frame::AsDouble() const
    {
        if (m_type.TypeID != TypeDouble) ESS_THROW(BadFieldType);

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        return reader.ReadDouble();
    }

    // ---------------------------------------------------------------
    
    void Frame::AsString(std::string &data) const
    {
        if (m_type.TypeID != TypeString) ESS_THROW(BadFieldType);

        data.clear();
        
        if (m_length == 0) return;

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);
        reader.ReadString(data, m_length);
    }

    // ---------------------------------------------------------------

    void Frame::AsWstring(std::wstring &data) const
    {
        if (m_type.TypeID != TypeWstring) ESS_THROW(BadFieldType);

        data.clear();
        
        if (m_length == 0) return;

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        if ((m_length % sizeof(wchar_t)) != 0) ESS_THROW(WStringOddSize);

        size_t size = m_length / sizeof(wchar_t);
        reader.ReadWstring(data, size);
    }

    // ---------------------------------------------------------------

    void Frame::AsBinary(std::vector<byte> &data) const
    {
        if (m_type.TypeID != TypeBinary) ESS_THROW(BadFieldType);

        data.clear();
        
        if (m_length == 0) return;

        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);
        reader.ReadVectorByte(data, m_length);
    }

    // ---------------------------------------------------------------

    int Frame::AsBinary(void *pData, size_t size) const
    {
        if (m_type.TypeID != TypeBinary) ESS_THROW(BadFieldType);
        if (size < m_length) ESS_THROW(BufferTooSmall);
        
        if (m_length == 0) return 0;
        
        Utils::MemReaderStream stream(m_pData, m_length);
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);
        reader.ReadData(pData, m_length);

        return m_length;
    }

    // ---------------------------------------------------------------

    std::string Frame::AsString() const
    {
        std::string res;
        AsString(res);

        return res;
    }
    
    // ---------------------------------------------------------------

    std::wstring Frame::AsWstring() const
    {
        std::wstring res;
        AsWstring(res);

        return res;
    }

    // ---------------------------------------------------------------

    std::vector<byte> Frame::AsBinary() const
    {
        std::vector<byte> res;
        AsBinary(res);

        return res;
    }

    // ---------------------------------------------------------------

    std::string Frame::ConvertToString() const
    {
        std::ostringstream res;
        res << "[Frame begin:" << std::endl
                << "|Type: " << m_type.TypeName << std::endl
                << "|Length: " << m_length << std::endl
                << "|Data: ";

        if (m_length == 0) res << "<empty>"; 
        else 
        {
            std::string data;
            m_type.ToStrConverter(m_pData, m_length, data);
            res << data;
        }
        
        res << std::endl << "Frame end]" << std::endl;
            
        return res.str();
    }

    std::string Frame::Value() const
    {
        std::string data;
        m_type.ToStrConverter(m_pData, m_length, data);

        return data;
    }

    std::string Frame::TypeAsString() const
    {
        return m_type.TypeName;
    }

    // ---------------------------------------------------------------

    bool Frame::SplitDataToFrames(const byte *pData, size_t size, 
                                  Utils::ManagedList<Frame> &outList)
    {
        ESS_ASSERT(pData != 0);
        ESS_ASSERT(size != 0);

        outList.Clear();

        size_t offset = 0;
        while (offset < size)
        {
            // parse header
            Utils::MemReaderStream stream((static_cast<const byte*>(pData) + offset),
                                          (size - offset));
            Utils::BinaryReader<Utils::MemReaderStream> reader(stream);
            FrameHeaderParser header;

            bool headerComplete = false;
            while (!headerComplete)
            {
                bool dataTypeIsInvalid = false;
                headerComplete = header.Add(reader.ReadByte(), dataTypeIsInvalid);

                if (dataTypeIsInvalid) return false;
            }

            dword frameLen = header.getLength();
            // ESS_ASSERT(frameLen != 0);

            // skip frame header
            offset += header.getHeaderLength();

            // add frame
            outList.Add(new Frame(header.getDataType(), 
                                   (static_cast<const byte*>(pData) + offset), 
                                   frameLen));
            // skip frame body
            offset += frameLen;
        }

        return true;
    }


} // namespace SBProto

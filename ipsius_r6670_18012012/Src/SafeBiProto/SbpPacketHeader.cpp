
#include "stdafx.h"

#include "SbpPacketHeader.h"
#include "SbpException.h"

#include "Utils/MemReaderStream.h"
#include "Utils/BinaryReader.h"

// ------------------------------------------------------------------

namespace SBProto
{
    // PacketHeader impl
    
    PacketHeader::PacketHeader(byte packType, byte protoVersion, word crc, dword dataLen)
    : m_packType(PackTypeFromByte(packType)), /* m_packType(packType), */
        m_protoVersion(protoVersion), m_crc(crc), m_dataLen(dataLen)
    {
    }

    // ------------------------------------------------------------------
    
    PacketHeader::SbpPacketType PacketHeader::PackTypeFromByte(byte b)
    {
        SbpPacketType res;
        
        switch (b)
        {
        case 0:
            res = PTCommand;
            break;
        case 1:
            res = PTResponce;
            break;
        case 2:
            res = PTInfo;
            break;
        default:
            ESS_THROW(InvalidPacketType);
        }
        
        return res;
    }
    
    // ------------------------------------------------------------------

    PacketHeader::SbpPacketType PacketHeader::getPackType() const 
    {
        return m_packType; 
    }

    // ------------------------------------------------------------------
    
    byte PacketHeader::getProtoVersion() const 
    {
        return m_protoVersion; 
    }

    // ------------------------------------------------------------------

    word PacketHeader::getCRC() const 
    { 
        return m_crc; 
    }

    // ------------------------------------------------------------------

    dword PacketHeader::getDataLength() const 
    { 
        return m_dataLen; 
    }

    // ------------------------------------------------------------------

    std::string PacketHeader::ToString() const
    {
        std::ostringstream stream;
        stream << "[Header begin:" << std::endl 
                << "|Type: " <<  (int)m_packType << std::endl 
                << "|Proto version: " << (int)m_protoVersion << std::endl 
                << "|CRC: " << m_crc << std::endl
                << "|Data length: " << m_dataLen << std::endl
                << "Header end]" << std::endl;

        return stream.str();
    }

    // ------------------------------------------------------------------

    void PacketHeader::WriteHeaderToStream(Utils::MemWriterDynStream &stream,
                                          SbpPacketType type, byte protoVersion, word crc, 
                                          dword dataLen)
    {
        Utils::BinaryWriter<Utils::MemWriterDynStream> writer(stream);
        byte packTypeAndProtoVer = (protoVersion << 2) | (type & 0xff) ;
        writer.WriteByte(packTypeAndProtoVer);
        writer.WriteWord(crc);
        SBPUtils::DynamicLengthWriter(writer, dataLen);
    }
        
    // ------------------------------------------------------------------
    // PacketHeaderParser impl
    
    PacketHeaderParser::PacketHeaderParser()
    : m_stream(PacketHeader::CMaxHeaderSizeBytes, true), m_writer(m_stream)
    {
    }

    // ------------------------------------------------------------------

    bool PacketHeaderParser::Add(byte b)
    {
        ESS_ASSERT(!IsComplete());

        if (m_stream.BufferSize() < PacketHeader::CHeaderSizeExceptLengthBytes)
        {
            m_writer.WriteByte(b);
        }
        else m_lengthParser.Add(b);
        
        return IsComplete();
    }

    // ------------------------------------------------------------------
    
    void PacketHeaderParser::Reset()
    {
        m_lengthParser.Reset();

        // reset stream
        if (m_stream.BufferSize() == 0) return;
        m_stream.DetachBuffer();
        // m_stream.CreateNewBufferAfterDetach(PacketHeader::CMaxHeaderSizeBytes);
        m_stream.setCapacity(PacketHeader::CMaxHeaderSizeBytes);
    }

    // ------------------------------------------------------------------

    PacketHeader PacketHeaderParser::GetHeader() const
    {
        ESS_ASSERT(IsComplete());
        
        Utils::MemReaderStream stream(m_stream.GetBuffer(), m_stream.BufferSize());
        Utils::BinaryReader<Utils::MemReaderStream> reader(stream);

        byte packTypeAndProtoVer = reader.ReadByte();
        byte packType = packTypeAndProtoVer & 3; // 00000011b
        byte protoVersion = packTypeAndProtoVer >> 2;
        word crc = reader.ReadWord();

        return PacketHeader(packType, protoVersion, crc, m_lengthParser.getLength()); // can throw
    }

    // ------------------------------------------------------------------

    /*
    size_t PacketHeaderParser::getHeadetSizeBytes() const
    {
        ESS_ASSERT(IsComplete());
        return (m_stream.BufferSize() + m_lengthParser.getLengthSizeBytes());
    }
    */
    

    

} // namespace SBProto

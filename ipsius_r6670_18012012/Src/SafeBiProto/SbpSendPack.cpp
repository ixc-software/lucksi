
#include "stdafx.h"
#include "SbpSendPack.h"
#include "SbpCRC.h"
#include "SbpFrameHeader.h"
#include "SbpPacketHeader.h"
#include "SbpPackInfo.h"

#include "Utils/BinaryReader.h"
#include "Utils/MemReaderStream.h"

namespace SBProto
{
    // SbpSendPack impl

    SbpSendPack::SbpSendPack(ISafeBiProtoForSendPack &protocol,
                             PacketHeader::SbpPacketType packType) 
    : m_protocol(protocol), m_packType(packType),
        m_dataStream(m_protocol.PacketStreamCapacity()), m_dataWriter(m_dataStream)
    {
    }

    // ----------------------------------------------------------

    SbpSendPack::~SbpSendPack()
    {
        // make header
        Utils::MemWriterDynStream headerStream(PacketHeader::CMaxHeaderSizeBytes);
        PacketHeader::WriteHeaderToStream(headerStream, m_packType, m_protocol.ProtoVersion(), 
                                         CountCRC(), m_dataStream.BufferSize());

        // send packet via protocol
        m_protocol.SendPacketToTransport(headerStream, m_dataStream, m_packType);
    }
    
    // ----------------------------------------------------------

    word SbpSendPack::CountCRC() const
    { 
        return CRC(m_dataStream.GetBuffer(), 0, m_dataStream.BufferSize()).get();
    }

    // ----------------------------------------------------------

    std::string SbpSendPack::ToString() const
    {
        PacketHeader header(m_packType, m_protocol.ProtoVersion(), CountCRC(), 
                            m_dataStream.BufferSize());
        SbpPackInfo packInfo(header, m_dataStream.GetBuffer(), m_dataStream.BufferSize());
        
        std::string res("Sent packet begin:");
        res += "\n";
        res += packInfo.ConvertToString();
        res += "Sent packet end.";
        res += "\n";

        return res;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteByte(byte data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeByte));
        m_dataWriter.WriteByte(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteBool(bool data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeBool));
        m_dataWriter.WriteBool(data); 
        return *this;
    }

    // ----------------------------------------------------------
    /*
    SbpSendPack& SbpSendPack::WriteChar(char data)
    {
        FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        m_dataWriter.WriteChar(data); 
        return *this;
    }
    */
    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteWord(word data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeWord));
        m_dataWriter.WriteWord(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteDword(dword data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeDword));
        m_dataWriter.WriteDword(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteInt32(int32 data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeInt32));
        m_dataWriter.WriteInt32(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteInt64(int64 data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeInt64));
        m_dataWriter.WriteInt64(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteFloat(float data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeFloat));
        m_dataWriter.WriteFloat(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteDouble(double data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeDouble));
        m_dataWriter.WriteDouble(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteString(const std::string &data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeString), data.size());
        m_dataWriter.WriteString(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteWstring(const std::wstring &data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeWstring), 
                          data.size() * sizeof(wchar_t));
        m_dataWriter.WriteWstring(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteBinary(const std::vector<byte> &data)
    {
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(data));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeBinary), data.size());
        m_dataWriter.WriteVectorByte(data); 
        return *this;
    }

    // ----------------------------------------------------------

    SbpSendPack& SbpSendPack::WriteBinary(const void *pData, size_t size)
    {
        // std::vector<byte> toGetType;
        // toGetType.resize(size);
        // FrameHeaderWriter(m_dataWriter, GetTypeInfo(toGetType));
        FrameHeaderWriter(m_dataWriter, m_typeTable.Find(TypeBinary), size);
        m_dataWriter.WriteData(pData, size); 
        return *this;
    }
   
    
} // namespace SBProto

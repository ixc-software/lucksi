
#ifndef __SBPSENDPACK__
#define __SBPSENDPACK__

// SbpSendPack.h

#include "SbpPacketHeader.h"
#include "ISbpProtoTransport.h"
#include "SbpGetTypeInfo.h"
#include "SbpFrame.h"
#include "Utils/MemWriterStream.h"
#include "Utils/BinaryWriter.h"
#include "Utils/ManagedList.h"
#include "Utils/ErrorsSubsystem.h"

namespace SBProto
{
    // Protocol interface used by SbpSendPack
    class ISafeBiProtoForSendPack : 
        public Utils::IBasicInterface
    {
    public:
        virtual byte ProtoVersion() const = 0;
        virtual size_t PacketStreamCapacity() const = 0;
        virtual void SendPacketToTransport(const Utils::MemWriterDynStream &header,
                                           const Utils::MemWriterDynStream &body, 
                                           PacketHeader::SbpPacketType packType) = 0;
    };

    // ------------------------------------------------------

    // Base class for all sent packets.
    // Send yourself via protocol in destructor
    class SbpSendPack:
        boost::noncopyable
    {
        ISafeBiProtoForSendPack &m_protocol;
        PacketHeader::SbpPacketType m_packType;
        
        Utils::MemWriterDynStream m_dataStream;
        Utils::BinaryWriter<Utils::MemWriterDynStream> m_dataWriter;

        TypeInfoTable m_typeTable;
        
        word CountCRC() const;

    public:
        SbpSendPack(ISafeBiProtoForSendPack &protocol, PacketHeader::SbpPacketType packType);
        virtual ~SbpSendPack();

        std::string ToString() const;

        // those methods can throw
        SbpSendPack& WriteByte(byte data);
        SbpSendPack& WriteBool(bool data);
        // SbpSendPack& WriteChar(char data);
        SbpSendPack& WriteWord(word data);
        SbpSendPack& WriteDword(dword data);
        SbpSendPack& WriteInt32(int32 data);
        SbpSendPack& WriteInt64(int64 data);
        SbpSendPack& WriteFloat(float data);
        SbpSendPack& WriteDouble(double data);
        SbpSendPack& WriteString(const std::string &data);
        SbpSendPack& WriteWstring(const std::wstring &data);
        SbpSendPack& WriteBinary(const std::vector<byte> &data);
        SbpSendPack& WriteBinary(const void *pData, size_t size);
    };

    // ------------------------------------------------------
    // Sent packets types

    class SbpSendPackCmd : public SbpSendPack
    {
    public:
        SbpSendPackCmd(ISafeBiProtoForSendPack &protocol) 
        : SbpSendPack(protocol, PacketHeader::PTCommand) 
        {
        }
    };

    // ------------------------------------------------------

    class SbpSendPackResp : public SbpSendPack
    {
    public:
        SbpSendPackResp(ISafeBiProtoForSendPack &protocol) 
        : SbpSendPack(protocol, PacketHeader::PTResponce)
        {
        }
    };

    // ------------------------------------------------------

    class SbpSendPackInfo : public SbpSendPack
    {
    public:
        SbpSendPackInfo(ISafeBiProtoForSendPack &protocol) 
        : SbpSendPack(protocol, PacketHeader::PTInfo)
        {
        }
    };

    
} // namespace SBProto

#endif

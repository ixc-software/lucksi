#ifndef SAFESERIALIZATOR_H
#define SAFESERIALIZATOR_H

#include "SafeBiProto/SbpSendPack.h"
#include "SafeBiProto/SbpPacketHeader.h"
#include "SafeBiProto/SbpSerialize.h"

namespace BfBootCore
{
    using Platform::byte;

    // Адаптер внутренних средств сериализации SafeBiProto.
    // TSerialized должен иметь методы Serialize(SBProto::PacketWrite&)/Serialize(SBProto::PacketRead& ),
    // где оператором <<  будут записаны/прочтены его поля.
    template<class TSerialized>
    class SafeSerializator : public SBProto::ISafeBiProtoForSendPack
    {
        boost::shared_ptr<Utils::ManagedMemBlock> m_binary;

        TSerialized& m_serialized;
        const byte m_protoVersion;
        const size_t m_streamCapacity;


        // ISafeBiProtoForSendPack
    private:        

        byte ProtoVersion() const { return m_protoVersion; }
        size_t PacketStreamCapacity() const { return m_streamCapacity; }

        void SendPacketToTransport(Utils::MemWriterDynStream &header,
            Utils::MemWriterDynStream &body, 
            SBProto::PacketHeader::SbpPacketType packType)
        {            
            Utils::MemWriterDynStream packStream(PacketStreamCapacity());
            packStream.Write(header);
            packStream.Write(body);

            m_binary = packStream.DetachBuffer();            
        }
    public:

        ESS_TYPEDEF(CantParse);

        SafeSerializator(TSerialized& serialized, byte protoVersion, size_t streamCapacity = 1024)
            : m_serialized(serialized),
            m_protoVersion(protoVersion),
            m_streamCapacity(streamCapacity)
        {}

        Utils::ManagedMemBlock& getBinary()
        {
            if (!m_binary)
            {
                SBProto::SbpSendPackInfo outPack(*this);
                SBProto::PacketWrite write(outPack);
                m_serialized.Serialize(write);
                
                //Utils::ManagedMemBlock* p = 0;
                //return *p; 
            }
            ESS_ASSERT(m_binary);
            ESS_ASSERT(m_binary->Size());
            return *m_binary;
        }

        void SerializeOwner(const Platform::byte* data, int size)
        {
            SBProto::PacketHeaderParser headRecv;            

            while(true)
            {
                ESS_ASSERT(size > 0 && "Not enough data for header");  // Exception ?              
                size--;
                if (headRecv.Add( *data++ )) break;
            }

            SBProto::PacketHeader header = headRecv.GetHeader();
            SBProto::SbpRecvPack recvPack(header);
            SBProto::ISbpRecvPack& recvIntf = recvPack;            

            size_t addedSize;
            if (!recvIntf.AddToPacketBody(data, size, addedSize)) ESS_THROW(CantParse);
            if (addedSize != size)  ESS_THROW(CantParse);                        

            SBProto::PacketRead read(recvPack, 0);
            m_serialized.Serialize(read);
        }

    };
} // namespace BfBootCore

#endif

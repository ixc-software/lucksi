
#ifndef __SAFEBIPROTO__
#define __SAFEBIPROTO__

// SafeBiProto.h

#include "ISbpProtoTransport.h"
#include "SbpSendPack.h"
#include "SbpRecvPack.h"
#include "SbpError.h"
#include "SbpSettings.h"
#include "SbpPacketHeader.h"
#include "ISpbProtoMonitor.h"
#include "SbpPackInfo.h"
#include "SbpTimeoutsProcessor.h"

#include "Utils/MemWriterStream.h"
#include "Utils/BinaryWriter.h"


namespace SBProto
{
    // --------------------------------------------------------------
    
    // Call-back interface to protocol owner
    class ISafeBiProtoEvents: public Utils::IBasicInterface
    {
    public:
        virtual void CommandReceived(boost::shared_ptr<SbpRecvPack> data) = 0;
        virtual void ResponseReceived(boost::shared_ptr<SbpRecvPack> data) = 0;
        virtual void InfoReceived(boost::shared_ptr<SbpRecvPack> data) = 0;
        virtual void ProtocolError(boost::shared_ptr<SbpError> err) = 0;
    };

    // --------------------------------------------------------------
     
    // Protocol
    // Note: Bind self to given transport
    class SafeBiProto:
        public ISbpTransportToProto,
        public ISafeBiProtoForSendPack, 
        public boost::noncopyable
    {
        ISafeBiProtoEvents &m_owner;
        ISbpProtoTransport &m_transport;
        SbpSettings m_settings;
        ISpbProtoMonitor *m_pMonitor;
        
        bool m_isActivated;
        bool m_isWaitingResponce;
        bool m_isNeedToSendResponce;

        SbpTimeoutsProcessor m_timers;
        
        dword m_lastProcessCall; // check we're processing
        

        PacketHeaderParser m_recvPackHeaderParser;
        boost::shared_ptr<SbpRecvPack> m_recvPack;
	private:        
        template<class TError>
        void SendErrorToOwner(TError* pErr)
        {
            m_recvPackHeaderParser.Reset();
            m_recvPack.reset();

            m_owner.ProtocolError(boost::shared_ptr<TError>(pErr));
        }

        void SendPacketToOwner();
        void SpecificOnPacketSendActions(PacketHeader::SbpPacketType packType);
        
        void CheckIsProcessing();
        bool CheckRecvPackTypeSizeProtoVertion();
        bool CheckRecvPackCRCAndDataTypes();
        size_t ParseHeader(const void *pData, size_t size, bool &completeInd);
		boost::shared_ptr<SbpError> AddToRecvPackBody(const void *pData, size_t size, size_t &addedSize);
		void SendPacketToTransport(const void *pData, size_t size);
    // ISbpTransportToProto impl
    private:
        void DataReceived(const void *pData, size_t size);

    // ISafeBiProtoForSendPack impl
    private:
        size_t PacketStreamCapacity() const { return  m_settings.getMaxReceiveSize(); }
        byte ProtoVersion() const { return m_settings.getCurrentProtoVersion(); }
        void SendPacketToTransport(const Utils::MemWriterDynStream &header,
                                   const Utils::MemWriterDynStream &body, 
                                   PacketHeader::SbpPacketType packType);

    public:
        SafeBiProto(ISafeBiProtoEvents &owner, ISbpProtoTransport &transport, 
                    const SbpSettings &settings, ISpbProtoMonitor *pMonitor = 0);
        ~SafeBiProto();

        // Activate when transport connected
        void Activate() { m_isActivated = true; }
        // Deactivate when transport disconnected or protocol error occur
        void Deactivate();

        bool IsActive() const { return m_isActivated; }
        bool IsReadyToSendNextCmd() const { return !m_isWaitingResponce; }
        bool IsNeedToSendResponce() const { return m_isNeedToSendResponce; }

        SbpSettings &SettingsForTest()  {   return m_settings;  }
        const SbpSettings &Settings() const {   return m_settings;  }

        // Need to be called at regular intervals: check timeouts
        void Process();
    };
    
} // namespace SBProto

#endif

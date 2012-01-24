#ifndef _SBP_CONNECTION_H_
#define _SBP_CONNECTION_H_

#include "iCore/MsgObject.h"
#include "iLog/iLogSessionCreator.h"
#include "SafeBiProto/SafeBiProto.h"
#include "SafeBiProto/ISbpTransport.h"
#include "SbpConnProfile.h"
#include "SbpLogSettings.h"
#include "ISbpConnection.h"

namespace SBProtoExt
{   
    using SBProto::SbpPackInfo;
    
    class SbpConnection : boost::noncopyable,
        public iCore::MsgObject,
		SBProto::ISafeBiProtoEvents,
        SBProto::ISpbProtoMonitor,
		SBProto::ISafeBiProtoForSendPack,
        public SBProto::ISbpTransportToUser,
        public ISbpConnection
    {
        typedef SbpConnection T;
    public:
		ESS_TYPEDEF(CreateError);
		SbpConnection(iCore::MsgThread &thread, SbpConnProfile &profile, 
			boost::shared_ptr<SbpLogSettings> logSettings = boost::shared_ptr<SbpLogSettings>(new SbpLogSettings())); // can throw
        ~SbpConnection();
		
    // SBProto::ISbpTransportToUser
    private:
        void TransportConnected();
        void TransportDisconnected(const std::string &desc);
        void TransportError(const std::string &err);

    // SBProto::ISpbProtoMonitor
    private:
        void OnSpbPacketIn(const SbpPackInfo &data);
		void OnSpbPacketOut(const Utils::MemWriterDynStream &header, 
			const Utils::MemWriterDynStream &body);
        void OnRawDataRecv(const void *pData, size_t size);

	// SBProto::ISafeBiProtoForSendPack
	private:
		Platform::byte ProtoVersion() const;
		size_t PacketStreamCapacity() const;
		void SendPacketToTransport(const Utils::MemWriterDynStream &header,
			const Utils::MemWriterDynStream &body, 
			SBProto::PacketHeader::SbpPacketType packType);

    // SBProto::ISafeBiProtoEvents
    private:
        void CommandReceived(boost::shared_ptr<SBProto::SbpRecvPack> data);
        void ResponseReceived(boost::shared_ptr<SBProto::SbpRecvPack> data);
        void InfoReceived(boost::shared_ptr<SBProto::SbpRecvPack> data);
        void ProtocolError(boost::shared_ptr<SBProto::SbpError> err);

    // ISbpConnection
    private:
        void Process();
        void BindUser(Utils::SafeRef<ISbpConnectionEvents>);
        void UnbindUser();
        void ActivateConnection();
        void DeactivateConnection();
        SBProto::ISafeBiProtoForSendPack &Proto();
        std::string TransportInfo() const;
		std::string Name() const;

    // msg wrapper SBProto::ISafeBiProtoEvents
    private:
        void onCommandReceived(boost::shared_ptr<SBProto::SbpRecvPack> data);
        void onResponseReceived(boost::shared_ptr<SBProto::SbpRecvPack> data);
        void onInfoReceived(boost::shared_ptr<SBProto::SbpRecvPack> data);
        void onProtocolError(boost::shared_ptr<SBProto::SbpError> err);

    // msg wrapper SBProto::ISbpTransportToUser
	private:
		void onTransportConnected();
		void onTransportDisconnected(const std::string &err);

    private:
		void ActivateSbp();
		void DeactivateSbp();

    private:

        struct CheckProfile
        {
            CheckProfile(SbpConnProfile &profile)
            {
                ESS_ASSERT(profile.IsValid());
            }
        };

        CheckProfile m_checkProfile;
		bool m_isActivatedByUser;
		bool m_isSbpActivated;
        Utils::SafeRef<ISbpConnectionEvents> m_user;
        Utils::SafeRef<ISpbRawDataCallback> m_rawDataCallback;
        boost::shared_ptr<SBProto::ISbpTransport> m_transport;
        boost::scoped_ptr<SBProto::SafeBiProto> m_sbpProto;
        boost::shared_ptr<SbpLogSettings> m_logSettings;
        boost::scoped_ptr<iLogW::LogSession> m_log;
        iLogW::LogRecordTag m_tagInfo;
        iLogW::LogRecordTag m_tagError;
        iLogW::LogRecordTag m_tagRecvPack;
        iLogW::LogRecordTag m_tagSendPack;
    };
} // namespace SBProto

#endif


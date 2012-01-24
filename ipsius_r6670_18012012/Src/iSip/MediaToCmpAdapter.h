#pragma once

#include "ISipMediaLine.h"
#include "MediaLineList.h"
#include "SdpUtils.h"
#include "iCmpExt/ICmpChannel.h"


namespace iSip
{
	class MediaToCmpAdapter : boost::noncopyable,
		private ISipMediaLineEvents
	{
		typedef boost::function<iCmpExt::ICmpChannel* ()> CmpVhannelResolver;

	public:

		MediaToCmpAdapter(MediaLineList &mediaLines,
			CmpVhannelResolver cmpResolver) :
			m_mediaLines(mediaLines),
			m_cmpResolver(cmpResolver)
		{
			MediaLineList::Manager(m_mediaLines).AddMediaLine(*this, CreateMediaLineProfile());
		}

		~MediaToCmpAdapter()
		{
			MediaLineList::Manager(m_mediaLines).DeleteMediaLine(*this);
		}

	// ISipMediaLineEvents
	private:

		void StartRtpSend(const char *remoteIpAddr, 
			unsigned int remoteRtpPort, 
			unsigned int remoteRtcpPort,
			const ML::CodecList &codecs)
		{
			Utils::HostInf addr(remoteIpAddr, remoteRtpPort);

			CmpChannelRef().StartSend(addr);
		}

		void StopRtpSend()
		{
			CmpChannelRef().StopSend();
		}

		void StartRtpReceive(const ML::CodecList &codecs)
		{
			CmpChannelRef().StartRecv();
		}

		void StopRtpReceive()
		{
			CmpChannelRef().StopRecv();
		}

	private:

		iCmpExt::ICmpChannel& CmpChannelRef()
		{
			iCmpExt::ICmpChannel *cmpChannel = m_cmpResolver();

			ESS_ASSERT (cmpChannel != 0);

			return *cmpChannel;
		}

		const iCmpExt::ICmpChannel& CmpChannelRef() const 
		{
			const iCmpExt::ICmpChannel *cmpChannel = m_cmpResolver();

			ESS_ASSERT (cmpChannel != 0);

			return *cmpChannel;
		}

	private:

		MediaLineProfile CreateMediaLineProfile() const 
		{
			const iCmpExt::ICmpChannel &cmpChannel = CmpChannelRef();

			iSip::MediaLineProfile profile;
			
			profile.LocalDirection = ML::DIRECTION_TYPE_SENDRECV;

			profile.MediaType = ML::MEDIA_TYPE_AUDIO;

			profile.TransportProtocolType = ML::PROTOCOL_TYPE_RTP_AVP;

			profile.PacketTime = 20;

			profile.CodecList.push_back(SdpUtils::BuildG711Codec(cmpChannel.UseAlaw()));

			Utils::HostInf addr(cmpChannel.ChannelRtpAddr());

			profile.LocalRtpTuple = 
				iSip::MediaIpAddress(iSip::SipTransportInfo::Udp, 
				boost::asio::ip::address::from_string(addr.Address()), 
				addr.Port()); 

			return profile;
		}

	private:
		MediaLineList &m_mediaLines;

		CmpVhannelResolver m_cmpResolver;
	};

}

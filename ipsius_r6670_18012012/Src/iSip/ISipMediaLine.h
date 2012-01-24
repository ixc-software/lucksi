#pragma once

#include "Utils/IBasicInterface.h"
#include "ConversationProfile.h"
#include "Sdp/SdpMediaLine.h"
#include "MediaIpAddress.h"
#include "SdpUtils.h"
#include "Utils/ErrorsSubsystem.h"

namespace resip	{	class SdpContents ;	}

namespace iSip
{

	// -------------------------------------------------------------------------------------

	struct MediaLineProfile 
	{
		typedef sdpcontainer::SdpMediaLine ML;

		MediaLineProfile()
		{
			LocalDirection = ML::DIRECTION_TYPE_INACTIVE;

			MediaType = ML::MEDIA_TYPE_NONE;
			
			TransportProtocolType = ML::PROTOCOL_TYPE_NONE;
			
			PacketTime = 0; 
		}

		ML::SdpDirectionType LocalDirection;

		ML::SdpMediaType MediaType;

		ML::SdpTransportProtocolType TransportProtocolType;

		int PacketTime;

		ML::CodecList CodecList;

		MediaIpAddress LocalRtpTuple;

		MediaIpAddress LocalRtcpTuple;

		SecureMediaProfile SecureMedia;

		resip::Data LocalSrtpSessionKey;

		resip::Data MyCertFingerprint;

		std::string ToString() const
		{
			std::ostringstream out;		
			out << "t: "
				<< ML::SdpTransportProtocolTypeToString(TransportProtocolType)
				<< "; d: " 
				<< ML::SdpDirectionTypeToString(LocalDirection)
				<< "; m: " 
				<< ML::SdpMediaTypeToString(MediaType)
				<< "; m: " 
				<< "; c: " 
				<< LocalRtpTuple.ToString()
				<< "; codecs: [" << SdpUtils::ToString(CodecList) << "];";

			return out.str();
		}


	};

	// -------------------------------------------------------------------------------------

	class ISipMediaLineEvents : Utils::IBasicInterface
	{

	public:
		
		typedef sdpcontainer::SdpMediaLine ML;

		virtual sdpcontainer::SdpMediaLine::SdpCryptoSuiteType 
			ProcessCryptoSettings(const sdpcontainer::SdpMediaLine::CryptoList &cryptoList, 
			resip::Data &cryptoKey)
		{
			return sdpcontainer::SdpMediaLine::CRYPTO_SUITE_TYPE_NONE;
		}

		virtual void SetRemoteSDPFingerprint(const resip::Data &fingerprint)
		{
			ESS_UNIMPLEMENTED;
		}

		virtual void StartDtlsClient(const char *remoteIpAddr, 
			unsigned int remoteRtpPort, 
			unsigned int remoteRtcpPort)
		{
			ESS_UNIMPLEMENTED;
		}

		virtual void StartRtpSend(const char *remoteIpAddr, 
			unsigned int remoteRtpPort, 
			unsigned int remoteRtcpPort,
			const ML::CodecList &codecs) = 0;

		virtual void StopRtpSend() = 0;

		virtual void StartRtpReceive(const ML::CodecList &codecs) = 0;

		virtual void StopRtpReceive() = 0;
	};

}


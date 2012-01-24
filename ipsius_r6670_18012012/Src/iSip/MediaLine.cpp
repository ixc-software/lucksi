#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "MediaLine.h"
#include "iSip/ConversationProfile.h"
#include "iSip/SdpUtils.h"

#include "Utils/AtomicTypes.h"
#include "Utils/IntToString.h"


namespace
{

	std::string GenerateMediaLineName()
	{
		static Utils::AtomicInt GNumber;

		return "MediaLine_" + Utils::IntToString(GNumber.FetchAndAdd(1));
	}

	// -------------------------------------------------------------------------------------

	unsigned int SrtpMasterKeyLen() {	return 30; }

	// -------------------------------------------------------------------------------------

	void SetupSrtpAttribute(bool secureMediaRequired,
		iSip::SecureMediaProfile::SecureMediaCryptoSuite suite,
		const resip::Data &getLocalSrtpSessionKeyBase64,
		resip::SdpContents::Session::Medium &medium)
	{

		resip::Data crypto;

		switch(suite)
		{
		case iSip::SecureMediaProfile::SRTP_AES_CM_128_HMAC_SHA1_32:
			crypto = "1 AES_CM_128_HMAC_SHA1_32 inline:" + getLocalSrtpSessionKeyBase64;
			medium.addAttribute("crypto", crypto);
			crypto = "2 AES_CM_128_HMAC_SHA1_80 inline:" + getLocalSrtpSessionKeyBase64;
			medium.addAttribute("crypto", crypto);
			break;
		case iSip::SecureMediaProfile::SRTP_AES_CM_128_HMAC_SHA1_80:
			crypto = "1 AES_CM_128_HMAC_SHA1_80 inline:" + getLocalSrtpSessionKeyBase64;
			medium.addAttribute("crypto", crypto);
			crypto = "2 AES_CM_128_HMAC_SHA1_32 inline:" + getLocalSrtpSessionKeyBase64;
			medium.addAttribute("crypto", crypto);
			break;
		}

		if (secureMediaRequired)
		{
			medium.protocol() = resip::Symbols::RTP_SAVP;
		}
		else
		{
			medium.protocol() = resip::Symbols::RTP_AVP;
			medium.addAttribute("encryption", "optional");  // Used by SNOM phones?
			medium.addAttribute("tcap", "1 RTP/SAVP");      // draft-ietf-mmusic-sdp-capability-negotiation-08
			medium.addAttribute("pcfg", "1 t=1");
		}
	}

	// -------------------------------------------------------------------------------------

	void SetupSrtpDtlsAttribute(bool secureMediaRequired,
		resip::SdpContents::Session::Medium &medium)
	{
		if (secureMediaRequired)
		{
			medium.protocol() = resip::Symbols::UDP_TLS_RTP_SAVP;
		}
		else
		{
			medium.protocol() = resip::Symbols::RTP_AVP;
			// draft-ietf-mmusic-sdp-capability-negotiation-08
			medium.addAttribute("tcap", "1 UDP/TLS/RTP/SAVP");
			medium.addAttribute("pcfg", "1 t=1");
			//medium.addAttribute("pcfg", "1 t=1 a=1");
		}
	}

	// -------------------------------------------------------------------------------------

	void SetupDirrection(sdpcontainer::SdpMediaLine::SdpDirectionType localDirection,
//		sdpcontainer::SdpMediaLine::SdpDirectionType remoteDirection,
		resip::SdpContents::Session::Medium &medium)
	{
		typedef sdpcontainer::SdpMediaLine ML;

		medium.clearAttribute("sendrecv");
		medium.clearAttribute("sendonly");
		medium.clearAttribute("recvonly");
		medium.clearAttribute("inactive");
/*
		if (remoteDirection == ML::DIRECTION_TYPE_INACTIVE)
		{
			localDirection = ML::DIRECTION_TYPE_INACTIVE;
		} else if (remoteDirection != ML::DIRECTION_TYPE_SENDRECV)
		{
			if (remoteDirection == localDirection) localDirection = ML::DIRECTION_TYPE_INACTIVE;
		}
*/
		medium.addAttribute(ML::SdpDirectionTypeToString(localDirection));
	}

	// -------------------------------------------------------------------------------------

	void CopyCodec(const sdpcontainer::SdpCodec &src,
		resip::SdpContents::Session::Codec &dst)
	{
		dst = resip::SdpContents::Session::Codec(
			src.getMimeSubtype(),
			src.getPayloadType(),
			src.getRate());
	}

	// -------------------------------------------------------------------------------------

	void CopyCodecsToMedium(
		const sdpcontainer::SdpMediaLine::CodecList &codecs,
		resip::SdpContents::Session::Medium &medium)
	{
		for(sdpcontainer::SdpMediaLine::CodecList::const_iterator i = codecs.begin();
			i != codecs.end(); ++i)
		{
			resip::SdpContents::Session::Codec codec;
			CopyCodec(*i, codec);
			medium.addCodec(codec);
		}
	}

}

namespace iSip
{

	MediaLine::MediaLine(iLogW::ILogSessionCreator &logCreator,
		ISipMediaLineEvents &media,
		const MediaLineProfile &profile) :
		m_media(media),
		m_profile(profile),
		m_log(logCreator.CreateSession(GenerateMediaLineName(), true)),
		m_infoTag(m_log->RegisterRecordKind(L"Info", true))
	{

		m_remoteDirection = ML::DIRECTION_TYPE_NONE;

		if(m_log->LogActive(m_infoTag))
		{
			*m_log  << m_infoTag << "Created. "
				<< profile.ToString()
				<< iLogW::EndRecord;
		}
	}

	// -------------------------------------------------------------------------------------

	bool MediaLine::AdjustRTPStreams( const sdpcontainer::SdpMediaLine &offer )
	{
		ESS_ASSERT(m_profile.MediaType == offer.getMediaType());

		ESS_ASSERT(m_profile.TransportProtocolType == offer.getTransportProtocolType());

		ML::CodecList resultCodecs;
		const ML::CodecList &localCodecs = m_profile.CodecList;

		if (!SdpUtils::CalculateCodecsIntersection(localCodecs,
			offer.getCodecs(),
			resultCodecs)) return false;

		ESS_ASSERT (!resultCodecs.empty());


		bool supportedCryptoSuite = false;

		resip::Data cryptoKey;

		if (SecureMediaMode() == SecureMediaProfile::Srtp ||
			offer.getTransportProtocolType() == ML::PROTOCOL_TYPE_RTP_SAVP)
		{
			supportedCryptoSuite = m_media.ProcessCryptoSettings(offer.getCryptos(), cryptoKey) !=
				ML::CRYPTO_SUITE_TYPE_NONE;
		}

		// Process Fingerprint and setup settings (if required)
		bool supportedFingerprint = IsFingerprintSupported(offer);

		if (SecureMediaRequired() && !supportedCryptoSuite && !supportedFingerprint)
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log  << m_infoTag << "Secure media is required and no valid support found in remote sdp - ending call." << iLogW::EndRecord;
			}

			return false;
		}

		// Aggregate local and remote direction attributes to determine overall media direction
		ML::SdpDirectionType mediaDirection =
			SdpUtils::CalculateDirectionType(m_profile.LocalDirection, offer.getDirection());

		m_remoteDirection = offer.getDirection();;

		unsigned int remoteRtpPort   = offer.getConnections().front().getPort();
		unsigned int remoteRtcpPort  = offer.getRtcpConnections().front().getPort();
		resip::Data  remoteIPAddress = offer.getConnections().front().getAddress();

		if (!offer.getFingerPrint().empty())
		{
			if(m_log->LogActive(m_infoTag))
			{
				*m_log  << m_infoTag << "Fingerprint retrieved from remote SDP: "
					<< SipUtils::ToString(offer.getFingerPrint())
					<< iLogW::EndRecord;
			}

			// ensure we only accept media streams with this fingerprint
			m_media.SetRemoteSDPFingerprint(offer.getFingerPrint());

			// If remote setup value is not active then we must be the Dtls client  - ensure client DtlsSocket is create
			if (offer.getTcpSetupAttribute() != sdpcontainer::SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE)
			{
				// If we are the active end, then kick start the DTLS handshake
				m_media.StartDtlsClient(remoteIPAddress.c_str(), remoteRtpPort, remoteRtcpPort);
			}
		}

		InitReceiveProcess(mediaDirection, resultCodecs);

		if (remoteRtpPort != 0 &&
			(mediaDirection == ML::DIRECTION_TYPE_SENDRECV ||
			 mediaDirection == ML::DIRECTION_TYPE_SENDONLY))
		{
			m_media.StartRtpSend(remoteIPAddress.c_str(), remoteRtpPort, remoteRtcpPort, resultCodecs);
			
			return true;
		}

		m_media.StopRtpSend();

		return true;
	}

	// -------------------------------------------------------------------------------------

	void MediaLine::InitReceiveProcess()
	{
		InitReceiveProcess(ML::DIRECTION_TYPE_SENDRECV, m_profile.CodecList);
	}

	// -------------------------------------------------------------------------------------

	void MediaLine::InitReceiveProcess( ML::SdpDirectionType mediaDirection, const ML::CodecList &codecs )
	{
		// No remote SDP info - so put direction into receive only mode (unless inactive)
		if (mediaDirection == ML::DIRECTION_TYPE_INACTIVE ||
			mediaDirection == ML::DIRECTION_TYPE_SENDONLY)
		{
			m_media.StopRtpReceive();

			return;
		}

		m_media.StartRtpReceive(codecs);
	}

	// -------------------------------------------------------------------------------------

	void MediaLine::BuildResipMedium(resip::SdpContents::Session::Medium &medium) const
	{
		medium = resip::SdpContents::Session::Medium(
			sdpcontainer::SdpMediaLine::SdpMediaTypeToString(m_profile.MediaType),
			m_profile.LocalRtpTuple.getPort(),
			0,
			sdpcontainer::SdpMediaLine::SdpTransportProtocolTypeToString(m_profile.TransportProtocolType));

		// Add Crypto attributes (if required) - assumes there is only 1 media stream
		medium.clearAttribute("crypto");
		medium.clearAttribute("encryption");
		medium.clearAttribute("tcap");
		medium.clearAttribute("pcfg");

		if (SecureMediaMode() == SecureMediaProfile::Srtp)
		{
			// Note:  We could add the crypto attribute to the "SDP Capabilties Negotiation"
			//        potential configuration if secure media is not required - but other implementations
			//        should ignore them any way if just plain RTP is used.  It is thought the
			//        current implementation will increase interopability. (ie. SNOM Phones)

			SetupSrtpAttribute(SecureMediaRequired(),
				SecureMediaCryptoSuite(),
				GetLocalSrtpSessionKey().base64encode(),
				medium);
		}

		if (SecureMediaMode() == SecureMediaProfile::SrtpDtls)
		{
			SetupSrtpDtlsAttribute(SecureMediaRequired(), medium);
		}

		CopyCodecsToMedium(m_profile.CodecList, medium);

		SetupDirrection(m_profile.LocalDirection, medium);
	}

	// -------------------------------------------------------------------------------------

	bool MediaLine::AnswerMediaLine(const sdpcontainer::SdpMediaLine &offer,
		resip::SdpContents::Session::Medium &answer,
		bool potential) const
	{
		if (m_profile.MediaType != offer.getMediaType()) return false;

		if (m_profile.TransportProtocolType != offer.getTransportProtocolType()) return false;

		if (offer.getConnections().empty()  ||
			offer.getConnections().front().getPort() == 0) return false;

		bool readyForReceive = m_profile.LocalDirection == ML::DIRECTION_TYPE_SENDRECV ||
			m_profile.LocalDirection == ML::DIRECTION_TYPE_RECVONLY;

		answer = resip::SdpContents::Session::Medium(
			sdpcontainer::SdpMediaLine::SdpMediaTypeToString(m_profile.MediaType),
			(!readyForReceive) ? 0 : m_profile.LocalRtpTuple.getPort(),
			0,
			sdpcontainer::SdpMediaLine::SdpTransportProtocolTypeToString(m_profile.TransportProtocolType));

		// Check secure media properties and requirements
		bool secureMediaRequired = SecureMediaRequired() ||
			m_profile.TransportProtocolType != sdpcontainer::SdpMediaLine::PROTOCOL_TYPE_RTP_AVP;

		if (SecureMediaMode() == SecureMediaProfile::Srtp ||
			m_profile.TransportProtocolType == sdpcontainer::SdpMediaLine::PROTOCOL_TYPE_RTP_SAVP)
		{
			// allow accepting of SAVP profiles, even if SRTP is not enabled as a SecureMedia mode

			bool supportedCryptoSuite =
				SetupSecureMediaModeToMedia(offer.getCryptos(), answer);

			if (!supportedCryptoSuite && secureMediaRequired)
			{
				if(m_log->LogActive(m_infoTag))
				{
					*m_log  << m_infoTag << "Secure media stream is required, but there is no supported crypto attributes in the offer - skipping this stream..."
						<< iLogW::EndRecord;
				}
				return false;
			}
		}

		if (SecureMediaMode() == SecureMediaProfile::SrtpDtls ||
			m_profile.TransportProtocolType == sdpcontainer::SdpMediaLine::PROTOCOL_TYPE_UDP_TLS_RTP_SAVP)
		{
			// allow accepting of DTLS SAVP profiles, even if DTLS-SRTP is not enabled as a SecureMedia mode

			bool supportedFingerprint = SetupDtlsSecureMediaMode(offer, answer);

			if (!supportedFingerprint && secureMediaRequired)
			{
				if(m_log->LogActive(m_infoTag))
				{
					*m_log  << m_infoTag << "Secure media stream is required, but there is no supported fingerprint attributes in the offer - skipping this stream..." << iLogW::EndRecord;
				}
				return false;
			}
		}

		ML::CodecList resultCodecs;

		if(!SdpUtils::CalculateCodecsIntersection(m_profile.CodecList,
			offer.getCodecs(),
			resultCodecs)) return false;

	
		ESS_ASSERT(!resultCodecs.empty());

		CopyCodecsToMedium(resultCodecs, answer);

		if (potential && !offer.getPotentialMediaViews().empty())
		{
			answer.addAttribute("acfg", offer.getPotentialMediaViewString());
		}

		// copy ptime attribute from session caps (if exists)

		if (m_profile.PacketTime != 0)
		{
			answer.addAttribute("ptime", resip::Data(m_profile.PacketTime));
		}

		// Check requested direction

		// Aggregate local and remote direction attributes to determine overall media direction
		ML::SdpDirectionType mediaDirection =
			SdpUtils::CalculateDirectionType(m_profile.LocalDirection, offer.getDirection());

		answer.addAttribute(sdpcontainer::SdpMediaLine::SdpDirectionTypeToString(mediaDirection));

		return true;
	}

	// -------------------------------------------------------------------------------------

	void MediaLine::BuildConnectionAttribute(iSip::ConversationProfile::NatIpConverter converter,
		resip::SdpContents::Session::Connection &connection) const
	{
		resip::SdpContents::AddrType ipVersion = m_profile.LocalRtpTuple.getAddress().is_v4() ?
			resip::SdpContents::IP4 :
			resip::SdpContents::IP6;


		resip::Data ip = m_profile.LocalRtpTuple.getAddress().to_string().c_str();

		if (!converter.empty()) ip = converter(ip);

		connection = resip::SdpContents::Session::Connection(ipVersion, ip);
	}

	// -------------------------------------------------------------------------------------

	bool MediaLine::IsFingerprintSupported( const sdpcontainer::SdpMediaLine &remoteMediaLine )
	{
		// Process Fingerprint and setup settings (if required)
		if (remoteMediaLine.getTransportProtocolType() !=
			sdpcontainer::SdpMediaLine::PROTOCOL_TYPE_UDP_TLS_RTP_SAVP) return false;

		if (remoteMediaLine.getFingerPrintHashFunction() == sdpcontainer::SdpMediaLine::FINGERPRINT_HASH_FUNC_NONE)
		{
			return false;
		}

		// We will only process Dtls-Srtp if fingerprint is in SHA-1 format
		if (remoteMediaLine.getFingerPrintHashFunction() != sdpcontainer::SdpMediaLine::FINGERPRINT_HASH_FUNC_SHA_1)
		{
			// "Fingerprint found, but is not using SHA-1 hash.";
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------

	bool MediaLine::SetupSecureMediaModeToMedia(
		const sdpcontainer::SdpMediaLine::CryptoList &cryptoList,
		resip::SdpContents::Session::Medium &medium) const
	{

		bool supportedCryptoSuite = false;
		for(sdpcontainer::SdpMediaLine::CryptoList::const_iterator i = cryptoList.begin();
			!supportedCryptoSuite && i != cryptoList.end(); ++i)
		{
			resip::Data cryptoKeyB64(i->getCryptoKeyParams().front().getKeyValue());
			resip::Data cryptoKey = cryptoKeyB64.base64decode();

			if (cryptoKey.size() != SrtpMasterKeyLen())
			{

				if(m_log->LogActive(m_infoTag))
				{
					*m_log  << m_infoTag << "SDES crypto key found in SDP, but is not of correct length after base 64 decode: "
						<< cryptoKey.size()
						<< iLogW::EndRecord;
				}

				continue;
			}

			switch(i->getSuite())
			{
			case sdpcontainer::SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_80:
				medium.addAttribute("crypto", resip::Data(i->getTag()) + " AES_CM_128_HMAC_SHA1_80 inline:" + GetLocalSrtpSessionKey().base64encode());
				supportedCryptoSuite = true;
				break;

			case sdpcontainer::SdpMediaLine::CRYPTO_SUITE_TYPE_AES_CM_128_HMAC_SHA1_32:
				medium.addAttribute("crypto", resip::Data(i->getTag()) + " AES_CM_128_HMAC_SHA1_32 inline:" + GetLocalSrtpSessionKey().base64encode());
				supportedCryptoSuite = true;
				break;
			}
		}

		return supportedCryptoSuite;
	}

	// -------------------------------------------------------------------------------------

	bool MediaLine::SetupDtlsSecureMediaMode(const sdpcontainer::SdpMediaLine &offer,
		resip::SdpContents::Session::Medium &medium) const
	{
		// We will only process Dtls-Srtp if fingerprint is in SHA-1 format
		if (offer.getFingerPrintHashFunction() != sdpcontainer::SdpMediaLine::FINGERPRINT_HASH_FUNC_SHA_1) return false;

		medium.clearAttribute("fingerprint");  // ensure we don't add these twice
		medium.clearAttribute("setup");  // ensure we don't add these twice

		// Add fingerprint attribute to sdp

		medium.addAttribute("fingerprint", "SHA-1 " + GetMyCertFingerprint());

		// Add setup attribute
		if (offer.getTcpSetupAttribute() == sdpcontainer::SdpMediaLine::TCP_SETUP_ATTRIBUTE_ACTIVE)
		{
			medium.addAttribute("setup", "passive");
		}
		else
		{
			medium.addAttribute("setup", "active");
		}

		return true;
	}
	
	// -------------------------------------------------------------------------------------
	// profile

	bool MediaLine::SecureMediaRequired() const
	{
		return m_profile.SecureMedia.SecureMediaRequired();
	}

	// -------------------------------------------------------------------------------------

	SecureMediaProfile::SecureMediaMode MediaLine::SecureMediaMode() const
	{
		return m_profile.SecureMedia.GetSecureMediaMode();
	}

	// -------------------------------------------------------------------------------------

	SecureMediaProfile::SecureMediaCryptoSuite MediaLine::SecureMediaCryptoSuite() const
	{
		return m_profile.SecureMedia.GetSecureMediaDefaultCryptoSuite();
	}

	// -------------------------------------------------------------------------------------

	resip::Data MediaLine::GetLocalSrtpSessionKey() const
	{
		return m_profile.LocalSrtpSessionKey;
	}

	// -------------------------------------------------------------------------------------

	resip::Data MediaLine::GetMyCertFingerprint() const
	{
		return m_profile.MyCertFingerprint;
	}

}

#pragma once

#include "ISipMediaLine.h"
#include "iSip/ConversationProfile.h"
#include "iSip/SipUtils.h"
#include "iSip/Sdp/SdpMediaLine.h"

#include "iLog/LogWrapper.h"

namespace sdpcontainer	{	class SdpMediaLine;	}
namespace resip			{	class SdpContents ;	}

namespace iSip
{

	class MediaLine : boost::noncopyable
	{
		typedef sdpcontainer::SdpMediaLine ML;
	
	public:

		MediaLine(iLogW::ILogSessionCreator &logCreator,
			ISipMediaLineEvents &media,
			const MediaLineProfile &profile);

		bool IsSame(const ISipMediaLineEvents &media) const { return	&m_media == &media; }

		const MediaLineProfile& Profile() const {	return m_profile; }
		MediaLineProfile& Profile()				{	return m_profile; }

	public:

		bool AdjustRTPStreams(const sdpcontainer::SdpMediaLine &remoteMediaLine);

		void InitReceiveProcess();

	// const method for sdp building 
	public:


		void BuildResipMedium(resip::SdpContents::Session::Medium &medium) const;

		bool AnswerMediaLine(const sdpcontainer::SdpMediaLine &offer,
			resip::SdpContents::Session::Medium &answer, 
			bool potential) const;

		void BuildConnectionAttribute(iSip::ConversationProfile::NatIpConverter converter,
			resip::SdpContents::Session::Connection &connection) const;

	// 
	private:

		void InitReceiveProcess(ML::SdpDirectionType mediaDirection,
			const ML::CodecList &codecs);

	private:

		static bool IsFingerprintSupported(const sdpcontainer::SdpMediaLine &remoteMediaLine);

	private:

		bool SetupSecureMediaModeToMedia(
			const sdpcontainer::SdpMediaLine::CryptoList &cryptoList,
			resip::SdpContents::Session::Medium &medium) const;

		bool SetupDtlsSecureMediaMode(const sdpcontainer::SdpMediaLine &offer,
			resip::SdpContents::Session::Medium &medium) const;

	// profile
	private:
		
		bool SecureMediaRequired() const;
	
		SecureMediaProfile::SecureMediaMode SecureMediaMode() const;

		SecureMediaProfile::SecureMediaCryptoSuite SecureMediaCryptoSuite() const;

		resip::Data GetLocalSrtpSessionKey() const;

		resip::Data GetMyCertFingerprint() const;

	private:

		boost::scoped_ptr<iLogW::LogSession>  m_log;
		iLogW::LogRecordTag m_infoTag;

		ISipMediaLineEvents &m_media;
		MediaLineProfile m_profile;

		ML::SdpDirectionType m_remoteDirection;
	};

}


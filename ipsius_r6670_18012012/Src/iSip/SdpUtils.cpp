#include "stdafx.h"
#include "iSip/undeferrno.h"
#include "SdpUtils.h"
#include "SipUtils.h"

#include "Utils/ErrorsSubsystem.h"


#include "rutil/Timer.hxx"

namespace
{
	const char* OriginatorName() {	return "ipsius"; }
}

namespace iSip
{

	typedef sdpcontainer::Sdp Sdp;

	sdpcontainer::SdpCodec
		SdpUtils::ConvertResipCodec(const resip::SdpContents::Session::Codec &resipCodec)
	{
		return sdpcontainer::SdpCodec(
			resipCodec.payloadType(),
			"",
			resipCodec.getName().c_str(),
			resipCodec.getRate(),
			20,
			resipCodec.encodingParameters().empty() ? 1 : resipCodec.encodingParameters().convertInt() /* Num Channels */,
			resipCodec.parameters().c_str());
	}

	// -------------------------------------------------------------------------------------

	sdpcontainer::SdpCodec SdpUtils::BuildG711Codec(bool alaw)
	{
		return (alaw) ?
			ConvertResipCodec(resip::SdpContents::Session::Codec::ALaw_8000) :
			ConvertResipCodec(resip::SdpContents::Session::Codec::ULaw_8000);
	}

	// -------------------------------------------------------------------------------------

	void SdpUtils::BuildSdpOffer(resip::SdpContents &offer)
	{
		SetupOrigin(offer);

		// Set local port in offer
		// for now we only allow 1 audio media
		ESS_ASSERT(offer.session().media().size() == 1);
		ESS_ASSERT(offer.session().media().front().name() == "audio");
	}

	// -------------------------------------------------------------------------------------

	void SdpUtils::SetupOrigin(resip::SdpContents &offer)
	{
		// Set sessionid and version for this offer
		UInt64 currentTime = resip::Timer::getTimeMicroSec();
		offer.session().origin().user() = "-";
		offer.session().origin().getSessionId() = currentTime;
		offer.session().origin().getVersion() = currentTime;
	}

	// -------------------------------------------------------------------------------------

	void SdpUtils::SetupOrigin(bool isIp4,
		const resip::Data &address,
		resip::SdpContents &offer)
	{
		SetupOrigin(offer);

		resip::SdpContents::AddrType addrType = (isIp4) ? resip::SdpContents::IP4 : resip::SdpContents::IP6;
		offer.session().origin().setAddress(address, addrType);
	}


	// -------------------------------------------------------------------------------------

	bool SdpUtils::CalculateCodecsIntersection(
		const SdpMediaLine::CodecList &localCodecs,
		const SdpMediaLine::CodecList &remoteCodecs,
		SdpMediaLine::CodecList &result)
	{
		typedef SdpMediaLine::CodecList CodecList;

		bool valid = false;

		CodecList::const_iterator itRemoteCodec = remoteCodecs.begin();
		for(; itRemoteCodec != remoteCodecs.end(); ++itRemoteCodec)
		{
			bool modeInRemote = itRemoteCodec->getFormatParameters().prefix("mode=");

			CodecList::const_iterator bestCapsCodecMatchIt = localCodecs.end();

			for(CodecList::const_iterator itLocalCodec = localCodecs.begin();
				itLocalCodec != localCodecs.end(); itLocalCodec++)
			{
				if (!isEqualNoCase(itRemoteCodec->getMimeSubtype(), itLocalCodec->getMimeSubtype()) ||
					itRemoteCodec->getRate() != itLocalCodec->getRate()) continue;

				bool modeInLocal = itLocalCodec->getFormatParameters().prefix("mode=");
				if (!modeInLocal && !modeInRemote)
				{
					// If mode is not specified in either - then we have a match
					bestCapsCodecMatchIt = itLocalCodec;
					valid = true;
					break;
				}

				if (modeInLocal && modeInRemote)
				{
					if (isEqualNoCase(itRemoteCodec->getFormatParameters(), itLocalCodec->getFormatParameters()))
					{
						bestCapsCodecMatchIt = itLocalCodec;
						valid = true;
						break;
					}
					// If mode is specified in both, and doesn't match - then we have no match
				}
				else
				{
					// Mode is specified on either offer or caps - this match is a potential candidate
					// As a rule - use first match of this kind only
					if (bestCapsCodecMatchIt == localCodecs.end()) bestCapsCodecMatchIt = itLocalCodec;
				}
			}

			if (bestCapsCodecMatchIt != localCodecs.end())
			{
				result.push_back(*bestCapsCodecMatchIt);

				if (isEqualNoCase(bestCapsCodecMatchIt->getMimeSubtype(), "telephone-event"))
				{
					// Consider offer valid if we see any matching codec other than telephone-event
					valid = true;
				}
			}
		}

		return valid;
	}

	// -------------------------------------------------------------------------------------

	sdpcontainer::SdpMediaLine::SdpDirectionType SdpUtils::CalculateDirectionType(
		SdpMediaLine::SdpDirectionType localDirection,
		SdpMediaLine::SdpDirectionType remoteDirection)
	{
		if (localDirection  == SdpMediaLine::DIRECTION_TYPE_INACTIVE ||
			remoteDirection == SdpMediaLine::DIRECTION_TYPE_INACTIVE)
		{
			return SdpMediaLine::DIRECTION_TYPE_INACTIVE;
		}

		if (localDirection == remoteDirection)
		{
			return (localDirection == SdpMediaLine::DIRECTION_TYPE_SENDRECV)	?
				SdpMediaLine::DIRECTION_TYPE_SENDRECV :
				SdpMediaLine::DIRECTION_TYPE_INACTIVE;
		}

		if (remoteDirection == SdpMediaLine::DIRECTION_TYPE_SENDONLY) return SdpMediaLine::DIRECTION_TYPE_RECVONLY;

		if (remoteDirection == SdpMediaLine::DIRECTION_TYPE_RECVONLY) return SdpMediaLine::DIRECTION_TYPE_SENDONLY;

		return localDirection;
	}

	// -------------------------------------------------------------------------------------

	std::string SdpUtils::ToString( const SdpMediaLine::CodecList &codecs )
	{
		std::ostringstream out;

		for (sdpcontainer::SdpMediaLine::CodecList::const_iterator i = codecs.begin();
			i != codecs.end(); ++i)
		{
			out << SipUtils::ToString(i->getMimeType()) << " ";
		}
		return out.str();
	}
}



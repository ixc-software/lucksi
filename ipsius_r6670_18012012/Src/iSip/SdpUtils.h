#pragma once

#include "resip/stack/SdpContents.hxx"
#include "iSip/Sdp/SdpMediaLine.h"

namespace iSip
{

	class SdpUtils
	{
		typedef sdpcontainer::SdpMediaLine SdpMediaLine;

	public:
		
		static sdpcontainer::SdpCodec ConvertResipCodec(
			const resip::SdpContents::Session::Codec &resipCodec);

		static sdpcontainer::SdpCodec BuildG711Codec(bool alaw);

		static void BuildSdpOffer(resip::SdpContents &offer);

		static void SetupOrigin(resip::SdpContents &offer);

		static void SetupOrigin(bool isIp4,
			const resip::Data &address,
			resip::SdpContents &offer);

		static bool CalculateCodecsIntersection(
			const SdpMediaLine::CodecList &localCodecs,
			const SdpMediaLine::CodecList &remoteCodecs,
			SdpMediaLine::CodecList &result);

		static SdpMediaLine::SdpDirectionType CalculateDirectionType(
			SdpMediaLine::SdpDirectionType localDirection,
			SdpMediaLine::SdpDirectionType remoteDirection);

		static std::string ToString(const SdpMediaLine::CodecList &codecs);

	};

}


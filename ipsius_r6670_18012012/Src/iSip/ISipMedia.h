#pragma once

#include "Utils/IBasicInterface.h"

namespace resip	
{ 
	class SdpContents; 
	class Data;
}

namespace sdpcontainer	{ class Sdp; }

namespace iSip 
{

	class ISipMedia : Utils::IBasicInterface
	{
	public:

		typedef boost::function<resip::Data (const resip::Data&)> NatIpConverter;

		virtual void SetupNatIpConverter(NatIpConverter converter) = 0;

		virtual void InitRemoteSdp(boost::shared_ptr<resip::SdpContents> offer) = 0;

		virtual void AdjustRTPStreams(bool isOffer) = 0;

		virtual bool GenerateAnswer(resip::SdpContents &answer) const = 0;

		virtual void GenerateOffer(resip::SdpContents &offer) const = 0;
	};

}




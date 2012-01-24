#pragma once

namespace Dss1ToSip
{
	class Dss1ToSipCauseConvertor
	{
	public:
		static int SipResponseToDss1Cause(int codeSip);
		static int Dss1CauseToSipResponse(int code);
	};
}


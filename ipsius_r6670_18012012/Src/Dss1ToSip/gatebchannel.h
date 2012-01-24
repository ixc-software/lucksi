#pragma once

#include "Utils/HostInf.h"
#include "Utils/WeakRef.h"

namespace Dss1ToSip
{
	class GateCallInfo;

	class GateBChannel 
	{
	public:
		GateBChannel(int channelNumber = 0) : m_channelNumber(channelNumber){}

		bool IsValid() const 
		{
			return m_channelNumber != 0;
		}
		int ChannelNumber() const 
		{
			return m_channelNumber;
		}
		void CmpActivated(const Utils::HostInf &boardRtp)
		{
			m_localRtp = boardRtp;
		}
		void CmpDeactivated()
		{
			m_localRtp.Clear();
		}

		Utils::HostInf LocalRtp() const
		{
			return m_localRtp;
		}

        void SaveCallInfo(Utils::WeakRef<GateCallInfo&> callInfo)
		{
			ESS_ASSERT(!m_callInfo.Valid());

			m_callInfo = callInfo;
		}

		Utils::WeakRef<GateCallInfo&> GetCallInfo() const
		{
			return m_callInfo;
		}
	private:
		int m_channelNumber;
		Utils::HostInf m_localRtp;
		Utils::WeakRef<GateCallInfo&> m_callInfo;
	};
}



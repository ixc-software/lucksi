#include "stdafx.h"
#include "SbpSettings.h"

namespace 
{
	using Platform::byte;
	using Platform::dword;
	const int  CPoolTimeout = 50;	
	const byte CCurrentProtoVersion = 1;
	const byte CMinProtoVersion = 1;
	const byte CMaxProtoVersion = 1;

	const dword CWaitResponceTimeoutMs = 60 * 1000;
	const dword CWaitReceiveEndTimeoutMs = 60 * 1000;
	const dword CTimeoutCheckIntervalMs = 10 * 1000; //

	const size_t CMaxSendSize = 1024;            // Send size limit (user to sbp)
	const size_t CMaxReceiveSize = 1024;         // Receive data size limit

}; 

namespace SBProto
{
	SbpSettings::SbpSettings() : m_currentProtoVersion(0), m_minProtoVersion(0), m_maxProtoVersion(0),
        m_maxSendSize(0), m_packSizeForTransport(0), m_responceTimeoutMs(0), m_timeoutCheckIntervalMs(0),
        maxReceiveSize(0)
    {
        setProtoVersions(CCurrentProtoVersion, CMinProtoVersion, CMaxProtoVersion);
        setMaxSendSize(CMaxSendSize);		
		setPoolTimeout(CPoolTimeout);
        setResponceTimeoutMs(CWaitResponceTimeoutMs);
        setReceiveEndTimeoutMs(CWaitReceiveEndTimeoutMs);
        setTimeoutCheckIntervalMs(CTimeoutCheckIntervalMs);
        setMaxReceiveSize(CMaxReceiveSize);
		
    }

} // namespace SBProto


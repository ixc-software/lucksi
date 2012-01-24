
#ifndef __SBPSETTINGS__
#define __SBPSETTINGS__


#include "Utils/ErrorsSubsystem.h"
#include "Platform/Platform.h"
#include "SbpTypes.h"


namespace SBProto
{
	using Platform::byte;
	using Platform::dword;

    // -------------------------------------------------------
    
    // Class - profile for SafeBiProto.
    // Timeouts are independent from check interval.
    // If timeouts set to 0, they are not processed.
    // If check interval set to 0, SafeBiProto::Process() doesn't need processing.
    class SbpSettings
    {
        byte m_currentProtoVersion;
        byte m_minProtoVersion;
        byte m_maxProtoVersion;

        size_t m_maxSendSize;
        size_t m_packSizeForTransport;
		int m_poolTimeout;
        dword m_responceTimeoutMs;
        dword m_receiveEndTimeoutMs; 
        dword m_timeoutCheckIntervalMs;

        size_t maxReceiveSize;
        
    public:
		// default value you can see in SbpSettings.cpp
		SbpSettings();
        void setMaxSendSize(size_t val) { m_maxSendSize = val; }        

		void setResponceTimeoutMs(dword val) 
        {
            // ESS_ASSERT((val > 0) && (val >= m_timeoutCheckIntervalMs));
            m_responceTimeoutMs = val; 
        }

        void setReceiveEndTimeoutMs(dword val) 
        { 
            // ESS_ASSERT((val > 0) && (val >= m_timeoutCheckIntervalMs));
            m_receiveEndTimeoutMs = val; 
        }
        
        void setTimeoutCheckIntervalMs(dword val) 
        { 
            // ESS_ASSERT(val > 0);
            m_timeoutCheckIntervalMs = val; 
        }
        
        void setProtoVersions(byte currentVer, byte minVer, byte maxVer)
        {
            ESS_ASSERT((currentVer >= minVer) && (currentVer <= maxVer) && (minVer <= maxVer));
            m_currentProtoVersion = currentVer;
            m_minProtoVersion = minVer;
            m_maxProtoVersion = maxVer;
        }

        void setMaxReceiveSize(size_t val)
        {
            maxReceiveSize = val;
        }
		void setPoolTimeout(int val) {	m_poolTimeout = val; }
		int getPoolTimeout() const	{	return m_poolTimeout;  }
        size_t getMaxSendSize() const { return m_maxSendSize; }		
        dword getResponceTimeoutMs() const { return m_responceTimeoutMs; }
        dword getReceiveEndTimeoutMs() const { return m_receiveEndTimeoutMs; }
        dword getTimeoutCheckIntervalMs() const { return m_timeoutCheckIntervalMs; }
        byte getCurrentProtoVersion() const { return m_currentProtoVersion; }
        byte getMaxProtoVersion() const { return m_maxProtoVersion; }
        byte getMinProtoVersion() const { return m_minProtoVersion; }
        size_t getMaxReceiveSize() const { return maxReceiveSize; }

        bool CheckProtoVersion(byte val) const 
        {
            return ((val >= m_minProtoVersion) && (val <= m_maxProtoVersion));
        }
    };
    
} // namespace SBProto

#endif 

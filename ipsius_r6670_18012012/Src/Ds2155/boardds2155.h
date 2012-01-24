#ifndef __DS2155_BRD__
#define __DS2155_BRD__

#include "stdafx.h"

#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "Utils/BidirBuffer.h"
#include "BfEmul/E1PcEmul.h"

#include "LiuPriStatus.h"
#include "HdlcProfile.h"
#include "HdlcStats.h"
#include "liuProfile.h"

namespace Ds2155
{
    using namespace Platform;

    enum 
    {
        CBaseAddress = 0x10000000,
    };
    
    class Liu
    {
    public:

        PriStatus PeekStatus()
        {
            PriStatus status;
            status.rxSignalLevel = 0;
            status.rxLOS = false;
            status.rxLOF = false;
            return status;
        }
    };
    
    // -------------------------------------------
    
    class Hdlc
    {
        std::string m_name;
        HdlcStatistic m_stats;

    public:

        Hdlc(const std::string &name)
        {
            m_name = name;
        }

        void SendPack(void *p, int size)
        {
            m_stats.packSended++;
            BfEmul::PcEmul::Instance().SendHdlcPack(m_name, p, size);
        }

        Utils::BidirBuffer* GetPack()
        {
            Utils::BidirBuffer *p = BfEmul::PcEmul::Instance().RecvHdlcPack(m_name);
            if (p != 0) m_stats.packRecieved++;
            return p;
        }

        ErrorFlags PeekErrors()
        {
            return ErrorFlags();
        }

        const HdlcStatistic &PeekStatistic() const
        {
            return m_stats;
        }

    };

    // -------------------------------------------
    
    class HdlcLogProfile
    {
    public:
        HdlcLogProfile(bool, int) { }
    };

    class ILogHdlc : public Utils::IBasicInterface
    {
    };
    
    // -------------------------------------------
                   
    class BoardDs2155 : boost::noncopyable
	{
        std::string m_name;
        Liu m_liu;
		
	public:
	    
		BoardDs2155(
            const std::string &name, 
			dword baseAddress,
		 	const LiuProfile &liuProfile,
			const HdlcProfile &hdlcProfile,
			ILogHdlc *log = 0) 	    
	    {
	        m_name = name;
	    }
	    
		Hdlc *AllocHdlc(byte channel, HdlcLogProfile logProfile = HdlcLogProfile(false, 0))
        {
            return new Hdlc(m_name);
        }
		 				
		void HdlcPolling()
		{
		    // nothing
		}

        Liu &LIU()
        {        
        	return m_liu;
        }
		
	};

	
};

#endif








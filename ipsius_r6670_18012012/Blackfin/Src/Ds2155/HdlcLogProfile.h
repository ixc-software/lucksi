#ifndef __DS2155_HDLC_LOG_PROFILE__
#define __DS2155_HDLC_LOG_PROFILE__

#include "stdafx.h"
#include "Platform\platformtypes.h"

namespace Ds2155
{
    using namespace Platform;

    /*
       Hdlc log profile
		.....
    */

    enum
    {
    	CTransivedTransActSize = 1<< 8,
    	CTxDumpPack = 1<<7,
    	CTxUserLevelEnabled = 1<<6,
		CTxPollingEnabled = 1<<5,
		CTxFSMEnabled = 1<<4,
		
    	CRxDumpPack = 1<<3,
    	CRxUserLevelEnabled = 1<<2,
		CRxPollingEnabled = 1<<1,
		CRxFSMEnabled = 1,
    };            
    
	class HdlcLogProfile 
	{
	public:
	    
	    HdlcLogProfile(bool enabled, int level):
    	m_isEnabled(enabled),    	
    	m_isTxDumpPack(level & CTxDumpPack),
    	m_isTxUserLevelEnabled(level & CTxUserLevelEnabled),
		m_isTxPollingEnabled(level & CTxPollingEnabled),
		m_isTxFSMEnabled(level & CTxFSMEnabled),		
    	m_isRxDumpPack(level & CRxDumpPack),
    	m_isRxUserLevelEnabled(level & CRxUserLevelEnabled),
		m_isRxPollingEnabled(level & CRxPollingEnabled),
		m_isRxFSMEnabled(level & CRxFSMEnabled),
		m_isTransivedTransActSize(level & CTransivedTransActSize)		
		{
		};							
			
		bool isAnyFlag()
		{
			bool flag = false;
			
	    	if(m_isEnabled) flag = true;
    	
	    	if(m_isTxDumpPack) flag = true;
	    	if(m_isTxUserLevelEnabled) flag = true;
			if(m_isTxPollingEnabled) flag = true;
			if(m_isTxFSMEnabled) flag = true;
		
	    	if(m_isRxDumpPack) flag = true;
	    	if(m_isRxUserLevelEnabled) flag = true;
			if(m_isRxPollingEnabled) flag = true;
			if(m_isRxFSMEnabled) flag = true;
			if(m_isTransivedTransActSize) flag = true;			
			
			return flag;
		}
		
    	bool m_isEnabled;
    	
    	bool m_isTxDumpPack;
    	bool m_isTxUserLevelEnabled;
		bool m_isTxPollingEnabled;
		bool m_isTxFSMEnabled;
		
    	bool m_isRxDumpPack;
    	bool m_isRxUserLevelEnabled;
		bool m_isRxPollingEnabled;
		bool m_isRxFSMEnabled;
		bool m_isTransivedTransActSize;		

	};

};

#endif

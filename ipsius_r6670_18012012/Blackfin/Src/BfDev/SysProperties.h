#ifndef _BF_SYS_PROPERTIES_H_
#define _BF_SYS_PROPERTIES_H_

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace BfDev
{
	using namespace Platform;	
	
	class SysProperties : public boost::noncopyable
		
	{
	public:	
	    static void InitFromProjectOptions();			    	    		
	    static void InitSysFreq118750kHz();			    
	    static void InitSysFreq133000kHz();	    

	    void VerifyInit() const { /* nothing */ }

	    static SysProperties &Instance();

	    void DisableDataCache();
	    void EnableDataCache();	    	    
	    	    
		std::string ToString() const;

        dword getFrequencyGen() const;  // ref freq
		dword getFrequencyCpu() const;  // gen x multi
        dword getFrequencySys() const;  // cpu / divider
		
		bool isDataCacheEnable() const;
		bool isDataInstructionEnable() const;
		bool isExceptions() const;

        void MemFlush(void *p, int size) const;
        void MemFlushInv(void *p, int size) const;
	
	private:		
	
	    static void InitMemAsync();	    	    	
		SysProperties(word pllDiv, word pllCtl);
		SysProperties();		

		void Calculation();
	    void SetSysFreq(word pllDiv, word pllCtl);				
	    
	private:		
		dword m_frequencyCpu;
		dword m_frequencyGen;
		dword m_frequencySys;
		
		int  m_countDisableDataCache;
		bool m_isDataCacheEnableBankA;
		bool m_isDataCacheEnableBankB;		
		bool m_isDataInstructionEnable;
		bool m_exceptions;
		
	};
	
    // RAII as push/pop with counter of deep
    class DisableDataCache 
    {
    public:
    	DisableDataCache()
    	{
		    SysProperties::Instance().DisableDataCache();
    	} 
    	
    	~DisableDataCache()
    	{
			SysProperties::Instance().EnableDataCache();    	
    	} 
    };

};

#endif

#include "stdafx.h"

#include "VDK.h"
#include <ccblkfn.h>
#include <cplbtab.h>

#include "Utils/ErrorsSubsystem.h"
#include "SysProperties.h"
#include "ds2155/ds2155hal.h"
#include "adi_ssl_init.h"

namespace BfDev
{
	using namespace Platform;							
    
	const char* BoolToState(bool val)
	{
		return val ? "Enabled" : "Disabled";
	}
	
	struct SysFreq18750kHz
	{
		static const word C_PLL_DIV = 0x0004;
        static const word C_PLL_CTL = 0x2600;
	};
	
	struct SysFreq133000kHz
	{
		static const word C_PLL_DIV = 0x0005;
        static const word C_PLL_CTL = 0x3000;
	};
	
	
	byte bufSysProperties[sizeof(BfDev::SysProperties)];
	BfDev::SysProperties *ptrSysProperties = 0;
		
	enum
    {
    	C_1MHZ = 1000000,
    };
    
    float IntToMHz(dword val)
    {
    	float z = val;
    	return z / C_1MHZ;
    }
    
    void WriteFreq(std::ostringstream &out, const char *pName, dword freq)
    {
	   out << pName << " = " << IntToMHz(freq) << " MHz\n";    	
    }

    void* AlignPtr(void *p)
    {
        return (void *)(((unsigned int)p)&(~31));
    }

    void* PtrEnd(void *p, int size)
    {
        char *pStart = (char*)p;
        return (pStart + size);
    }
    	
};

namespace BfDev
{

	SysProperties::SysProperties(word pllDiv, word pllCtl) : 
		m_frequencyGen(25000000),
		m_countDisableDataCache(0)
	{
		SetSysFreq(pllDiv, pllCtl);
		Calculation();
		InitMemAsync();
		adi_ssl_Init();
	}
	
	//---------------------------------------------------				
		
	SysProperties::SysProperties() : 
		m_frequencyGen(25000000),
		m_countDisableDataCache(0)
	{
		Calculation();
		InitMemAsync();
		adi_ssl_Init();
	}
	
	//---------------------------------------------------					
	
    void SysProperties::InitMemAsync()
    {        
		// Bank 0,1,2 for Flash memory, bank 3 for DS2155	    	
		*pEBIU_AMGCTL = 0x00FA;
		*pEBIU_AMBCTL1 = (*pEBIU_AMBCTL1 & 0x0000FFFF) | 0xffc20000;
		ssync();      
    }
	
	//---------------------------------------------------			
		
	void SysProperties::SetSysFreq(word pllDiv, word pllCtl)
    {
        iVDK::CriticalRegion criticalRegion;
        *pPLL_DIV = pllDiv; 
        *pPLL_CTL = pllCtl; 
        idle();
    }

	//---------------------------------------------------	
	
	void SysProperties::InitFromProjectOptions()
	{
		ESS_ASSERT(!ptrSysProperties);
		
		ptrSysProperties = new(bufSysProperties) SysProperties;
	}
	
	//---------------------------------------------------	
	
	void SysProperties::InitSysFreq118750kHz()
	{
		ESS_ASSERT(!ptrSysProperties);
		
		ptrSysProperties = new(bufSysProperties) 
			SysProperties(SysFreq18750kHz::C_PLL_DIV, SysFreq18750kHz::C_PLL_CTL);
	}
	
	//---------------------------------------------------	
	
	void SysProperties::InitSysFreq133000kHz()
	{
		ESS_ASSERT(!ptrSysProperties);
		
		ptrSysProperties = new(bufSysProperties) 
			SysProperties(SysFreq133000kHz::C_PLL_DIV, SysFreq133000kHz::C_PLL_CTL);
	}


	
	//---------------------------------------------------	
	
	SysProperties &SysProperties::Instance()
	{
		ESS_ASSERT(ptrSysProperties);
		
		return *ptrSysProperties;
	}
	
	//--------------------------------------------		
    
	
	void SysProperties::DisableDataCache()
	{
		if(!m_countDisableDataCache)
			disable_data_cache(); 		
		m_countDisableDataCache++;
	}
    
	//--------------------------------------------		
	
	void SysProperties::EnableDataCache()
	{
		ESS_ASSERT(m_countDisableDataCache);
				
		m_countDisableDataCache--;
		
		if(m_countDisableDataCache)
			return;
		
		if(m_isDataCacheEnableBankB)
			enable_data_cache(CPLB_ENABLE_DCACHE2 | CPLB_ENABLE_DCACHE);	
		else if(m_isDataCacheEnableBankA)
			enable_data_cache(CPLB_ENABLE_DCACHE);			
	}
	
	//--------------------------------------------	
	
	void SysProperties::Calculation()
	{
	 	unsigned int t = (*pPLL_DIV) & 0x0F;

		m_frequencyCpu = *pPLL_CTL;//Частота поцессора (вычисляется)		  

		if(m_frequencyCpu & 1) m_frequencyGen /= 2;

		m_frequencyCpu >>= 9;
		m_frequencyCpu &= 0x3F;
		if(m_frequencyCpu == 0) 
			m_frequencyCpu = m_frequencyGen * 64; 
		else 
			m_frequencyCpu *= m_frequencyGen;
	
		m_frequencySys = m_frequencyCpu/t;
		
		word dataCache = ((*pDMEM_CONTROL) >> 2) & 0x03;	
		
		m_isDataCacheEnableBankA = (dataCache == 0x02);
		m_isDataCacheEnableBankB = (dataCache == 0x03);		
			
		m_isDataInstructionEnable = (*pIMEM_CONTROL) & 4;	
	 		 	
		m_exceptions = false;		
		#if (__EXCEPTIONS == 1)
		  m_exceptions = true;			
		#endif
		
		ESS_ASSERT( (VDK::GetTickPeriod() == 1) && "We must use tick period 1 ms.");
		
		VDK::SetClockFrequency(m_frequencyCpu/C_1MHZ);
	}
		
	//--------------------------------------------
	
	std::string SysProperties::ToString() const
	{
		std::ostringstream out;
			
		WriteFreq(out, "Fqen", getFrequencyGen());
		WriteFreq(out, "Fcpu", getFrequencyCpu());
		WriteFreq(out, "Fsys", getFrequencySys());

		out << "Data cache: " << BoolToState(isDataCacheEnable()) << std::endl;
		out << "Instruction cache: " << BoolToState(isDataInstructionEnable()) << std::endl;
		out << "Exceptions: " << BoolToState(isExceptions()) << std::endl;
		
		return out.str();
	}
	
	
	// --------------------------------------------
	
	dword SysProperties::getFrequencyCpu() const
	{
		return m_frequencyCpu;
	}	
	
	// --------------------------------------------
	
	dword SysProperties::getFrequencyGen() const
	{
		return m_frequencyGen;
	}	
	
	// --------------------------------------------
	
	dword SysProperties::getFrequencySys() const	
	{
		return m_frequencySys;
	}	
	
	// --------------------------------------------
	
	bool SysProperties::isDataCacheEnable() const
	{	
		return m_isDataCacheEnableBankA || m_isDataCacheEnableBankB;
	}	
	
	// --------------------------------------------

	bool SysProperties::isDataInstructionEnable() const
	{
		return m_isDataInstructionEnable;
	}	

	// --------------------------------------------
	
	bool SysProperties::isExceptions() const
	{
		return m_exceptions;
	}		

	// --------------------------------------------	

    void SysProperties::MemFlush(void *p, int size) const
    {
        if (!isDataCacheEnable()) return;

        void *start = AlignPtr(p);
        void *end   = PtrEnd(p, size);

        ssync();

        while (start < end)
        {
            {asm volatile("FLUSH[%0++];":"+p"(start));}
        }

        ssync();
    }

    void SysProperties::MemFlushInv(void *p, int size) const
    {
        if (!isDataCacheEnable()) return;

        void *start = AlignPtr(p);
        void *end   = PtrEnd(p, size);

        ssync();

        while (start < end)
        {
            {asm volatile("FLUSHINV[%0++];":"+p"(start));}
        }

        ssync();
    }

	
};

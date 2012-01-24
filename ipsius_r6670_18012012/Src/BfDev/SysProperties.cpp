#include "stdafx.h"

#include "Utils/ErrorsSubsystem.h"
#include "SysProperties.h"

namespace BfDev
{
	using namespace Platform;							
    	
	byte bufSysProperties[sizeof(BfDev::SysProperties)];
	BfDev::SysProperties *ptrSysProperties = 0;
		    	
};

namespace BfDev
{

		
	SysProperties::SysProperties() : 
		m_frequencyGen(25000000),
		m_countDisableDataCache(0)
	{
	}
	
	// ---------------------------------------------------	
	
	void SysProperties::InitFromProjectOptions()
	{
		ESS_ASSERT(!ptrSysProperties);
		
		ptrSysProperties = new(bufSysProperties) SysProperties;
	}
	
		
	// ---------------------------------------------------	
	
	SysProperties &SysProperties::Instance()
	{
		ESS_ASSERT(ptrSysProperties);
		
		return *ptrSysProperties;
	}
			
	// --------------------------------------------
	
	std::string SysProperties::ToString() const
	{
		std::ostringstream out;
					
		return out.str();
	}
	
	
	// --------------------------------------------
	
	dword SysProperties::getFrequencyCpu() const
	{
		return 1000000;
	}	
	
	// --------------------------------------------
	
	dword SysProperties::getFrequencyGen() const
	{
		return getFrequencyCpu();
	}	
	
	// --------------------------------------------
	
	dword SysProperties::getFrequencySys() const	
	{
		return getFrequencyCpu();
	}	
	
	
};

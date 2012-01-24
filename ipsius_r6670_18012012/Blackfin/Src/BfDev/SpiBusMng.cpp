#include "stdafx.h"

#include "SysProperties.h"

#include "SpiBusMng.h"


namespace BfDev
{

    // -----------------------------------------------------------------------

    SpiBusMng::SpiBusMng(int busSpeedHz) : 
        m_spi(BfDev::SysProperties::Instance().getFrequencySys(), busSpeedHz),
		m_speedHz(0)
    {		
    }    
    
    // -----------------------------------------------------------------------

    SpiBusMng::~SpiBusMng()
    {
    }        
        
    // -----------------------------------------------------------------------

    BfDev::BfSpi &SpiBusMng::GetBfSpi()
    {
        return m_spi;
    }    
    
    // -----------------------------------------------------------------------    
    
        
  	int GetSpiBaud(int speedHz)
    {
    	int freq = BfDev::SysProperties::Instance().getFrequencySys();
    	return BfDev::BfSpi::SpiBaud(freq, speedHz);
    }

    // -----------------------------------------------------------------------

    BfSpi& SpiBusPoint::Spi() { return m_spi->GetBfSpi(); }

    const BfSpi& SpiBusPoint::Spi() const { return m_spi->GetBfSpi(); }

    
} // namespace DevIpTdm

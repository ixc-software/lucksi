#include "stdafx.h"

#include <cplbtab.h> 
#include <cdefBF537.h>
#include <ccblkfn.h>

#include "BfDev/BfUartSimpleCore.h"
#include "BfDev/BfFreqInit.h"
#include "ExtFlash/FlashTest.h"

// -----------------------------------------------

class Ram : public ExtFlash::IFlashTestIntf
{
	enum 
	{ 
		CRamSize = 32*1024*1024,
		CPageSize = 256,
		
		CTotalPages = CRamSize / CPageSize,
	};
	
	ExtFlash::FlashMap m_map;
	
	static Platform::byte* PageAddr(int page)
	{
		ESS_ASSERT(page < CTotalPages);
		
		return (Platform::byte*)(page * CPageSize);
	}

// IFlashTestIntf impl	
private:

	const ExtFlash::FlashMap& Map() const
    {
    	return m_map;
    }
    
    bool SectorErase(int sectorNumber)
    {
    	enum { CFillVal = 0xff };
    	ESS_ASSERT(sectorNumber == 0);
    	
    	std::memset(0, 0xff, CRamSize);
    	
    	// verify
    	Platform::byte *p = 0;
    	for(int i = 0; i < CRamSize; ++i) 
    	{
    		ESS_ASSERT(p[i] == CFillVal);
    	}
    	
    	return true;
    }
    
    bool PageRead(int page, void *p, int size)
    {
    	ESS_ASSERT(size == CPageSize);
    	
    	std::memcpy(p, PageAddr(page), size);
    	
    	return true;
    }
    
    bool PageWrite(int page, const void *p, int size)
    {
    	ESS_ASSERT(size == CPageSize);
    	
    	std::memcpy(PageAddr(page), p, size);
    	
    	int cmp = std::memcmp(PageAddr(page), p, size);
    	ESS_ASSERT(cmp == 0);
    	    	
    	return true;
    }
	
public:

	Ram() : m_map(1, CRamSize, CPageSize)
	{
		// RAM init
		*pEBIU_SDRRC = 0x03A0;
		ssync();		
		*pEBIU_SDBCTL = 0x13;
		ssync();		
		*pEBIU_SDGCTL = 0x0091998D;
		ssync();			
	}
	
};

// -----------------------------------------------

class RamTest : public ExtFlash::IFlashTestCallback
{
	enum { CUartSpeed = 115200 };
	
	bool m_cpuInit;
	int m_frequencyCpu, m_frequencySys; 
	BfDev::BfUartSimpleCore m_uart;	
	Ram m_ram;
	int m_writeErrors, m_readErrors;
	
	bool CpuInit()
	{
		BfDev::BfFreqInit::FreqInit120MHz(m_frequencyCpu, m_frequencySys);		
	
		// data cache - ?!
		// ... 
		
		return true;
	}
	
	void Log(const std::string &s)
	{
		// cout
		std::cout << s << std::endl;
		
		// uart
		m_uart.Send(s.c_str());
		m_uart.Send(13);
		m_uart.Send(10);		
	}


// IFlashTestCallback impl
private:	

	void SectorErased(int sectorNumber)
	{
		// nothing
	}
	
    void PageWrited(int pageNumber, bool withError)
    {
    	if (pageNumber == 0) Log("Begin write");
    	if (withError) ++m_writeErrors;
    }
    
    void PageVerified(int pageNumber, bool withError)
    {
    	if (pageNumber == 0) Log("Begin verify");    	
    	if (withError) ++m_readErrors;    	
    }
		
public:

	RamTest() : 
		m_cpuInit(CpuInit()),
		m_uart(m_frequencySys, 0, CUartSpeed)
	{
	}	
	
	void Run()
	{
		Log("Test started!");
		
		// init
		m_writeErrors = 0; 
		m_readErrors = 0;
		
		// run
		ExtFlash::RunFullFlashTest(m_ram, *this);
		
		// output -- led vs UART
		std::ostringstream oss;
		oss << "Done, write err " << m_writeErrors << ", read err " << m_readErrors;
		Log(oss.str());
	}
	
};
 
// -----------------------------------------------

int main( void )
{
	// RAM 	
	{
		RamTest test;
		test.Run();		
	}
	
	while(true);
	
	return 0;
}

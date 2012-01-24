
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>

#include <cplbtab.h> 
#include <cdefBF537.h>
#include <ccblkfn.h>

#include "Platform/PlatformTypes.h"
#include "Platform/PlatformBlackfinMini.h"
#include "Platform/BfMiniAssert.h"
#include "BfDev/BfUartSimpleCore.h" 

using Platform::dword;
using Platform::byte;

// -------------------------------------------------------- 

//#pragma linkage_name _BootFromSpiTempAsm
void BootFromSpiAsm(unsigned long spiOffs, unsigned short devider);

void BootFromSpi(int spiOffs, int devider)
{
	// disable data cache
	disable_data_cache();
		
	// disable instruction cache
	volatile dword* pReg = (volatile dword*)IMEM_CONTROL;				
	dword mask = 0xfffffffd;
	*pReg = *pReg & mask;	
			
	BootFromSpiAsm(spiOffs, devider);
}						

// -------------------------------------------------------- 

enum LedColor
{
	LedOff = 0,
	LedGreen,
	LedRed,
	LedYellow,
};

void SetColorLed(LedColor color)
{    	
	*pPORTGIO_DIR		|= 0x0030;		// LEDs    	
	*pPORTGIO_CLEAR		= 0x0030;                    	    	
	
	if (color == LedYellow) *pPORTGIO_SET= 0x0030;                    	      
	if (color == LedRed)	*pPORTGIO_SET= 0x0020;
	if (color == LedGreen)	*pPORTGIO_SET= 0x0010;
}

// -------------------------------------------------------- 

enum
{
	CBusFreq = 120 * 1000 * 1000,
};

void FreqInit(int pllDiv, int pllCtl, int &frequencyCpu, int &frequencySys)
{
	const int CFrequencyGen = 25000000;
	
	*pPLL_DIV = pllDiv; 
    *pPLL_CTL = pllCtl; 
    idle();	
    
    // info
	int t = (*pPLL_DIV) & 0x0F;

	int frequencyGen = CFrequencyGen;
	frequencyCpu = *pPLL_CTL;
	if (frequencyCpu & 1) frequencyGen /= 2;

	frequencyCpu >>= 9;
	frequencyCpu &= 0x3F;
	
	if(frequencyCpu == 0) frequencyCpu = frequencyGen * 64; 
	else                  frequencyCpu *= frequencyGen;
	
	frequencySys = frequencyCpu / t;    
	
	ESS_ASSERT(frequencySys == CBusFreq);
}

void FreqInit120MHz(int &frequencyCpu, int &frequencySys)
{
	const int C_PLL_DIV = 0x0005;
    const int C_PLL_CTL = 0x3000;
    
	FreqInit(C_PLL_DIV, C_PLL_CTL, frequencyCpu, frequencySys);    
}

		
// -------------------------------------------------------- 
 
#define X16(m) m m m m m m m m m m m m m m m m

#define MEGABYTE(m) X16(X16(X16(X16(m))))

#define MSG0 "0123456789abcde0"
#define MSG1 "1123456789abcde1"
#define MSG2 "2123456789abcd_2"
#define MSG3 "3123456789abc__3"
#define MSG4 "0123456789abcde4"
#define MSG5 "1123456789abcde5"
#define MSG6 "2123456789abcd_6"
#define MSG7 "3123456789abc__7"

char *GMsg0 = MSG0;

char *GData0 = MEGABYTE(MSG0);
char *GData1 = MEGABYTE(MSG1);
char *GData2 = MEGABYTE(MSG2);
char *GData3 = MEGABYTE(MSG3);

char *GData4 = MEGABYTE(MSG4);
char *GData5 = MEGABYTE(MSG5);
char *GData6 = MEGABYTE(MSG6);
char *GData7 = MEGABYTE(MSG7);


const int CMsgSize   = 16;
const int CBlockSize = 1024 * 1024;

// -------------------------------------------------------- 

int StringLen(const char *p)
{
	int count = 0;
	while(*p++) ++count;
	return count;
}

// -------------------------------------------------------- 

class Test
{
	int m_busFreq, m_cpuFreq;
	bool m_freqInit;		
	BfDev::BfUartSimpleCore m_uart;
	
	enum { CUartSpeed = 115200 };
		
	bool FreqInit()
	{
		FreqInit120MHz(m_cpuFreq, m_busFreq);
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
	
	static int BlockVerify(const char *pBlock, int blockSize, 
	                const char *pVerify, int verifySize,
	                int &firstErrorAddr)
	{
		int verifyIndx = 0;
		int errors = 0;

		for(int i = 0; i < blockSize; ++i)
		{
			if (pBlock[i] != pVerify[verifyIndx]) 
		    {
		    	if (errors == 0) firstErrorAddr = i;
		    	++errors;
		    }

			++verifyIndx;
			if (verifyIndx >= verifySize) verifyIndx = 0;
		}
		
		return errors;
	}
	
	static void Halt()
	{
		// led
		SetColorLed(LedRed);
		ESS_HALT("");		
	}
	
	template<class T>		
	static std::string AddrToStr(T p)
	{
		BOOST_STATIC_ASSERT(sizeof(T) == sizeof(dword));
		dword addr = (dword)p;
		std::ostringstream oss;
		oss << std::showbase << std::hex << addr;
		return oss.str();
	}
	
	void DumpBlock(const char *pBlock, int count, const char *prefix)
	{
		std::ostringstream oss;
		oss << prefix << " @ " << AddrToStr(pBlock) << "; " << std::showbase << std::hex;
		
		for(int i = 0; i < count; ++i)
		{
			int val = (byte)pBlock[i];
			oss << val << " ";
		}
		
		Log(oss.str());
	}
		
	void DoVerifyBlock(const char *pBlock, const char *pVerify, const char *pMsg)
	{
		DumpBlock(pBlock,  16, pMsg);
		DumpBlock(pVerify, 16, "Verify");			
		
		int firstErrorAddr = 0;
		int errors = BlockVerify(pBlock, CBlockSize, pVerify, CMsgSize, firstErrorAddr);
			
		if (errors > 0)
		{
			using namespace std;
			ostringstream oss;
			oss << pMsg << " errors " << errors << " first " << showbase << hex << firstErrorAddr;
			Log(oss.str());
			
			Halt();
		}
	}
	
	static void Delay(int count)
	{
		while(count--)
		{
			int firstErrorAddr = 0;
			int errors = BlockVerify(GData0, CBlockSize, GMsg0, CMsgSize, firstErrorAddr);			
		}		
	}
			
	void RunVerify()
	{
		const bool CEnableBugs = false;
		
		if (CEnableBugs)
		{
			GData0[10] = 12;  						
			GData0[13] = 12;  			
		}
		
		DoVerifyBlock(GData0, GMsg0, "GData0");			
		// DoVerifyBlock(GData1, MSG1, "GData1");			
		// DoVerifyBlock(GData2, MSG2, "GData2");			
		// DoVerifyBlock(GData3, MSG3, "GData3");							
	}
	
	void SpiBoot()
	{
		const int CSpiDiv = 6;
		
		Log("Boot!");
		
		SetColorLed(LedOff);
		Delay(2);
		
		BootFromSpi(0, CSpiDiv);				
	}

	
	void LogMemoryInfo()
	{
		std::ostringstream oss;
		
		oss << "Method addr " << AddrToStr(&Test::Delay)
		    << " Stack addr " << AddrToStr(&oss);

		Log(oss.str());		
	}
	
	bool VerifyRAM()
    {
    	int errors = 0;
    	
    	for(int i = 0; i < 16; ++i)
    	{
    		char val = GData0[i];
    		GData0[i] = i;
    		if (GData0[i] != i) ++errors;
    		GData0[i] = val;
    	}
    	
    	return (errors == 0);
    }


	
public:

	Test() : 
		m_freqInit(FreqInit()),
		m_uart(m_busFreq, 0, CUartSpeed)
	{
		SetColorLed(LedGreen);
		
		Log("");
	
		enum { CMHz = 1000 * 1000 };
		std::ostringstream oss;
		oss << "CPU " << m_cpuFreq / CMHz << " MHz ; bus " << m_busFreq / CMHz << " MHz";
		Log(oss.str());
		
		LogMemoryInfo();
		
		if (!VerifyRAM()) Log("RAM test fail!");
		
		{
			int val = *pEBIU_SDBCTL;
			std::ostringstream oss;
			oss << "pEBIU_SDBCTL " << std::showbase << std::hex << val;
			Log(oss.str());
		}
	}
	
	void Run()
	{
		RunVerify();
		
		SpiBoot();
	}
	
};


// -------------------------------------------------------- 
 
int main( void )
{
	using namespace std;
	
	Test t;
	t.Run();
			
	return 0;
}

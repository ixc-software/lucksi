#include "stdafx.h"
#include <VDK.h>
#include "iostream.h"

#include "Utils/TimerTicks.h"
#include "Platform/PlatformTypes.h"
#include "BfDev/SysProperties.h"

namespace
{
	using namespace Platform;
	
	class Tests
	{
		static const dword XMASLEN = 500000;		
		static const dword m_testTimeout = 500;//Время выполнения теста mc
		static const dword m_countCicle = 1000;	 		
		
		typedef dword (*FnTest)(dword);		
		dword m_frequencyProc;		
	public:
	
		Tests(dword frequencyProc) : m_frequencyProc(frequencyProc) // / m_countCicle / (1000 / m_testTimeout) )
		{}
		void RunAll();
		
	private:		
	
		void Loop(const char *str, FnTest fn, dword count = m_countCicle)
		{
			std::cout<< str;
			unsigned int i = 0;
		    dword d;			
		    
		    Utils::TimerTicks timer;
		    timer.Set(m_testTimeout, false);

		    while(!timer.TimeOut())
			{
				i++;
				d = (*fn)(count);				
			}
			
			float result = m_frequencyProc / count;
			result /= (1000 / timer.Get());
			result /= i;
			
			std::cout << result << std::endl;
		}
	
		static dword TestProcRegSUM(dword s);
		static dword TestProcMemSUM(dword s);
		static dword TestProcMemMasRead(dword s);
		static dword TestProcMemMasRWrite(dword s);
		static dword TestProcLargeMasRWrite(dword s);
		static dword TestProcRegMUL(dword s);
		static dword TestProcRegDIV(dword s);
		static dword TestProcCALL(dword s);
		static dword TestProcJUMP(dword s);
		static dword ExceptionsPayloadLoop(dword s);
	};
		
	//---------------------------------------

	dword Tests::TestProcMemMasRead(dword s)
	{
		static dword m_test[m_countCicle];								
		s = m_countCicle;
		
		dword s_res = 0;
		while(s--)
		{
			asm("nop;");
			s_res += m_test[s];
		}
		return s_res;
	}

	//---------------------------------------

	dword Tests::TestProcMemMasRWrite(dword s)
	{
		static dword m_test[m_countCicle];
		s = m_countCicle;
		dword s_res = 0;
		while(s--)
		{
			asm("nop;");
			m_test[s] += s;
		}
		return s_res;
	}
	
	//---------------------------------------
	
	dword Tests::TestProcLargeMasRWrite(dword s)
	{
		static dword m_Xtst[XMASLEN];							
		dword s_res = 0;
		s = XMASLEN;
		while(s--)
		{
			asm("nop;");
			m_Xtst[s] += s;
		}
		return s_res;
	}
	
	//---------------------------------------	
	
	volatile bool GDoThrow = false;
    volatile int  GDestructionCounter;

    class ConstructorCanThrow
    {
    public:
        ConstructorCanThrow()
        {
        	#if (__EXCEPTIONS == 1)
              if (GDoThrow) throw std::exception();
            #endif
        }

        ~ConstructorCanThrow()
        {
            GDestructionCounter++;
        }
    };

    void ExceptionsPayloadTest()
    {
        ConstructorCanThrow a;
        ConstructorCanThrow b;
        ConstructorCanThrow c;
    }

	//---------------------------------------
	
	dword Tests::ExceptionsPayloadLoop(dword s)
	{
		while(s--)
		{
			asm("nop;");
			ExceptionsPayloadTest();
		}

		return 0;
	}

	//---------------------------------------
	
	dword Tests::TestProcJUMP(dword s)
	{
		while(s--)
		{
			if(s + 1) goto m1;
			asm("nop;");

	m1:		asm("nop;");
		}
		return s;
	}

	//---------------------------------------
	
	dword Tests::TestProcRegSUM(dword s)
	{
		dword s_res = 0;
		while(s--)
		{
			asm("nop;");
			s_res += s;
		}
		return s_res;
	}

	//---------------------------------------
	
	dword Tests::TestProcMemSUM(dword s)
	{
		dword s_res = 0;
		static volatile dword sum;		
		while(s--)
		{
			asm("nop;");
			sum += s;
		}
		return sum;
	}

	//---------------------------------------
	
	dword Tests::TestProcRegMUL(dword s)
	{
		dword m_res = 1;
		while(s--)
		{
			asm("nop;");
			m_res *= s;
		}
		return m_res;
	}

	//---------------------------------------
	
	void TestCall(void)
	{

		asm("nop;");

	}

	dword Tests::TestProcCALL(dword s)
	{
		while(s--)
		{
			TestCall();
		}
		return s;
	}

	//---------------------------------------
	
	dword Tests::TestProcRegDIV(dword s)
	{
		dword d_res = s;
		while(s--)
		{

			asm("nop;");

			d_res /= 3;

		}
		return d_res;
	}

	//---------------------------------------
	
	bool AddressOutsideRAM(void *pAddr)
	{
		int i = (int)pAddr;  // brutal
		return (i >= 0x20000000);
	}
	
	//---------------------------------------

	void Sleep(int timeout)
	{
	    Utils::TimerTicks timer;
	    timer.Set(timeout, false);
		while(!timer.TimeOut());
	}
	
	//---------------------------------------		
			
	void Tests::RunAll()
	{
	    std::cout << "Test\n";
	    
	    std::cout << "Address of function TestProcRegSUM = "
	    	<< (dword)&TestProcRegSUM << std::endl;        
		
	    Loop("Reg SUM         = ", &TestProcRegSUM);
		Loop("Mem SUM         = ", &TestProcMemSUM);
		Loop("Mem Array Read  = ", &TestProcMemMasRead);
		Loop("Mem Array R/W   = ", &TestProcMemMasRWrite);
		Loop("Large Array R/W = ", 
			&TestProcLargeMasRWrite, XMASLEN);
			
		Loop("Reg MUL         = ", &TestProcRegMUL);
		Loop("Reg DIV         = ", &TestProcRegDIV);
		Loop("Proc CALL       = ", &TestProcCALL);
		Loop("JUMP            = ", &TestProcJUMP);
		Loop("Except. Payload = ", &ExceptionsPayloadLoop);	

	}
	
	
	void TestTimer(Platform::dword timeout)
	{
	    Platform::word leds[6]    = {PF6, PF7, PF8, PF9, PF10, PF11};
	    Platform::word buttons[4] = {PF2, PF3, PF4, PF5};

		*pPORTFIO_DIR  |= (leds[0] | leds[1] | leds[2] | leds[3] | leds[4] | leds[5]);
		*pPORTFIO_INEN  = (buttons[0] | buttons[1] | buttons[2] | buttons[3]);
		*pPORTFIO_CLEAR = (leds[0] | leds[1] | leds[2] | leds[3] | leds[4] | leds[5]);
		
		std::cout << std::cout;
		std::cout << "Press any key and start timer." << std::cout;
		
	    *pPORTFIO ^= leds[0];		
	    
	    while (true)
	    {
	    	if (*pPORTFIO & (buttons[0] | buttons[1] | buttons[2] | buttons[3]))
	        	break;
	    }
    
	    *pPORTFIO ^= leds[1];
		Sleep(timeout);
		*pPORTFIO_CLEAR = leds[1];		
	}	
	
};

namespace BfBenchmark
{
	void Run()
	{	
		
		BfDev::SysProperties::InitFromProjectOptions();
		//BfDev::SysProperties::InitSysFreq118750kHz();
		
		std::cout << BfDev::SysProperties::Instance().ToString();
/*
		// тестирование таймера
		while(true)
		{
			TestTimer(100);
		}
*/		
	    Sleep(10);

		Tests(BfDev::SysProperties::Instance().getFrequencyCpu()).RunAll();
		
	    std::cout << "Done!" << endl;
	}	  
};

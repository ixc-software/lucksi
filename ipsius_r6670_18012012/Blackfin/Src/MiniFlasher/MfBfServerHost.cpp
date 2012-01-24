#include "stdafx.h"

#include "Utils/DelayInit.h"

#include "DevIpTdm/DevIpTdmSpiMng.h"
#include "DevIpTdm/BootFromSpi.h"
#include "DevIpTdm/BfLed.h"

#include "ExtFlash/M25P128.h"
#include "ExtFlash/FlashTest.h"
#include "ExtFlash/M25P128_test.h"

#include "BfDev/AdiDeviceManager.h"
#include "BfDev/BfUartSimple.h"

#include "iUart/BfUart.h"

#include "UtilsTests/ZlibPipeTest.h"
#include "Utils/IntToString.h"

#include "MfServer.h"
#include "MfBfServerHost.h"

// ----------------------------------------

namespace
{
    using namespace MiniFlasher;

    class UartTransport : public ITransport
    {    	    	
        BfDev::BfUartSimple m_uart;
        
       	int m_recvCounter;
       	int m_sendCounter;


    // ITransport impl
    public:

        void Send(const void *pData, int dataSize)
        {
        	if (dataSize > 0)
            {	
            	m_sendCounter += dataSize;
            }
        	
            m_uart.Write(pData, dataSize);
        }

        int Recv(void *pData, int buffSize)
        {
            return m_uart.Read(pData, buffSize);
        }

    public:

        UartTransport() :
        	m_uart(BfDev::SysProperties::Instance().getFrequencySys(), 0, 115200)
        {
        	m_recvCounter = 0;
        	m_sendCounter = 0;
        }
        
    };

    // ----------------------------------------

    class AsyncUartTransport : public ITransport
    {    	    
        BfDev::AdiDeviceManager m_devs;
        iUart::BfUartProfile m_profile;
        iUart::BfUart m_uart;
        iUart::IUartIO &m_io;

    // ITransport impl
    public:

        void Send(const void *pData, int dataSize)
        {
            m_io.Write(pData, dataSize);
        }

        int Recv(void *pData, int buffSize)
        {
            int count = m_io.Read(pData, buffSize);

            if (count > 0)
            {
                static int i;  // debug 
                i++;
            }

            return count;
        }

    public:

        AsyncUartTransport() :
            m_devs(1),
            m_profile(0, 256, 512, 115200, iUart::DataBits_8),
            m_uart(m_devs, m_profile),
            m_io( m_uart.IO() )
        {
        }

    };

    // -----------------------------------------

    class FlashTest : ExtFlash::IFlashTestCallback
    {
        MiniFlasher::IFlashTestCallback &m_callback;
        ExtFlash::FlashMap  m_map;
        bool m_silentMode;

        bool m_firstCycle;
        bool m_readyToSecondCycle;
        int m_readErrors;
        int m_writeErrors;

        int m_prevPercent;

        // 100% = 2 x cycle
        // cycle = Erase% + Write% + Verify%

        enum
        {
            CErasePercent   = 34,   // 130 sec
            CWritePercent   = 53,   // 200 sec
            CVerifyPercent  = 13,   // 50  sec
        };

        BOOST_STATIC_ASSERT( (CErasePercent + CWritePercent + CVerifyPercent) == 100);

        void Log(const std::string &msg)
        {
            if (m_silentMode) return;

            float time = Platform::GetSystemTickCount() / 1000.0;
            std::cout << time << ": " << msg << std::endl;
        }

        // limit to 99%
        int PercentFix(int percent)
        {
            ESS_ASSERT(percent <= 100);

            ESS_ASSERT(m_prevPercent <= percent);
            m_prevPercent = percent;

            return (percent == 100) ? 99 : percent;
        }
        
    // ExtFlash::IFlashTestCallback impl
    private:

        void SectorErased(int sectorNumber)
        {
            if (m_firstCycle && m_readyToSecondCycle)
            {
                m_firstCycle = false;
                Log("Second cycle begin!");
            }

            if (sectorNumber == 0) Log("Begin erase");

            int percent = (sectorNumber * 100) / m_map.Sectors;  // 0 .. 100%
            percent = percent * CErasePercent / 100;             // 0 .. CErasePercent
            percent = (percent + 0) / 2;                                     
            if (!m_firstCycle) percent += 50;

            m_callback.FlashTestState(PercentFix(percent), m_readErrors, m_writeErrors);
        }

        void PageWrited(int pageNumber, bool withError)
        {
            if (m_firstCycle && pageNumber == 0)
            {
                m_readyToSecondCycle = true;
            }

            if (pageNumber == 0) Log("Begin write");

            if (withError) ++m_writeErrors;

            int percent = (pageNumber * 100) / m_map.Pages;      // 0 .. 100%
            percent = percent * CWritePercent / 100;             // 0 .. CWritePercent
            percent = (percent + CErasePercent) / 2;                                       
            if (!m_firstCycle) percent += 50;
                                                             
            m_callback.FlashTestState(PercentFix(percent), m_readErrors, m_writeErrors);
        }

        void PageVerified(int pageNumber, bool withError)
        {
            if (pageNumber == 0) Log("Begin verify");

            if (withError) ++m_readErrors;

            int percent = (pageNumber * 100) / m_map.Pages;      // 0 .. 100%
            percent = percent * CVerifyPercent / 100;            // 0 .. CVerifyPercent
            percent = (percent + CErasePercent + CWritePercent) / 2;
            if (!m_firstCycle) percent += 50;

            m_callback.FlashTestState(PercentFix(percent), m_readErrors, m_writeErrors);           
        }

    public:

        FlashTest(ExtFlash::IFlashTestIntf &flash, 
                  MiniFlasher::IFlashTestCallback &callback, 
                  bool silentMode) :
            m_callback(callback), 
            m_map(flash.Map()),
            m_silentMode(silentMode)
        {
            m_firstCycle = true;
            m_readyToSecondCycle = false;  // set true after first write callback
            m_readErrors = 0;
            m_writeErrors = 0;

            m_prevPercent = 0;

            Log("Started!");
            ExtFlash::RunFullFlashTest(flash, *this);

            // final callback with 100% 
            m_callback.FlashTestState(100, m_readErrors, m_writeErrors);

            Log("Completed!");
        }

    };

    // -----------------------------------------

    class Flash : public IFlashInterface
    {
        DevIpTdm::DevIpTdmSpiMng m_spiBusMng;
        ExtFlash::M25P128	m_flash;                
        ExtFlash::FlashMap  m_map;
        ExtFlash::M25P128Test m_flashTestIntf;

        dword               m_eraseFirstSector;
        dword               m_erasedCount;
        
        dword               m_writePos;


    // IFlashInterface impl
    private:

        dword FlashSize() const
        {
            return m_map.TotalBytes;
        }

        dword FlashWriteGranularity() const
        {
            return m_map.PageSize;
        }

        bool EraseBegin(dword offs)
        {
        	m_eraseFirstSector = offs / m_map.SectorSize;
        	m_erasedCount = 0;
        	
			// offset must be sector aligned 
        	if (m_eraseFirstSector * m_map.SectorSize != offs) return false;
        	
            return true;
        }

        bool EraseNext(dword &erasedSize)
        {
        	int sectorToErase = m_eraseFirstSector + m_erasedCount;
            if (!m_map.CorrectSector(sectorToErase)) return false;

            m_flash.SectorErase(sectorToErase);
            ++m_erasedCount;
            erasedSize = m_erasedCount * m_map.SectorSize;

            return true;
        }

        void BeginWrite(dword offs)
        {
            m_writePos = offs;
        }

        bool Write(const void *pData, int size)
        {
            // verify: write to erased area
            int erasedBegin = m_eraseFirstSector * m_map.SectorSize;
            int erasedEnd = (m_eraseFirstSector + m_erasedCount) * m_map.SectorSize;
            if (m_writePos < erasedBegin) return false;
            if (m_writePos + size > erasedEnd) return false;

            // align check
            if (!m_map.CorrectForWrite(m_writePos, size, false)) return false;

            // write
            if (m_flash.Write(m_writePos, pData, size, true))
            {
                m_writePos += size;
                return true;
            }

            return false;
        }

        bool Read(void *pData, int offset, int size)
        {
            if (!m_map.CorrectForRead(offset, size)) return false;

            m_flash.Read(offset, pData, size);

            return true;
        }

        void RunFlashTest(MiniFlasher::IFlashTestCallback &callback)
        {
            FlashTest test(TestInterface(), callback, true);
        }

    public:

        Flash() :
          m_spiBusMng(),
		  m_flash( m_spiBusMng.CreatePoint( m_spiBusMng.GetFlashBusLock() ), true ),        
          m_map(m_flash.GetMap()),
          m_flashTestIntf(m_flash)
        {
        	m_eraseFirstSector = 0;
        	m_erasedCount = 0;        
        	m_writePos = 0;      
        }

        ExtFlash::IFlashTestIntf& TestInterface()
        {
            return m_flashTestIntf;
        }
        
        ExtFlash::M25P128& Driver()
        {
        	return m_flash;
        }
    };

    // -----------------------------------------

    class SysInfra : public ISysInterface
    {
    public:

        void Sleep()
        {
            // nothing
        }

        void Run(dword offs)
        {
            DevIpTdm::BootFromSpi( offs, BfDev::GetSpiBaud() );
        }

        void Leds(bool on)
        {
            DevIpTdm::BfLed::Set(0, on);	    	
            DevIpTdm::BfLed::Set(1, on);	    	
        }

    };

    // -----------------------------------------

    void FlashNotFound()
    {    	
        while(true)
        {
            DevIpTdm::BfLed::SetColor(DevIpTdm::RED);	
            iVDK::Sleep(300);
            
            DevIpTdm::BfLed::SetColor(DevIpTdm::OFF);	        	
            iVDK::Sleep(300);	        	
        }
        
        ESS_ASSERT(0 && "M25P128 not found");
    }
    
    // -----------------------------------------

    void UartSendString(ITransport &uart, const std::string &s)
    {
        if (s.empty()) return;

        uart.Send(s.c_str(), s.size());
    }

    void UartEcho(ITransport &uart)
    {
    	std::cout << "Echo test!" << std::endl;    	
		UartSendString(uart, "Echo!");    	
		
		while(true)
		{
			byte val;
			int recv = uart.Recv(&val, sizeof(val));
			if (recv == 0) continue;

			{
				std::ostringstream oss;
				oss << (int)val << " " << Platform::GetSystemTickCount() << "; ";
				UartSendString(uart, oss.str());				
			}			
		}
    }    
    
    // -----------------------------------------
    
    void StartupLeds()
    {    	
    	DevIpTdm::BfLed::Set(0, true);	    	
    	DevIpTdm::BfLed::Set(1, true);	    	
    	
    	Platform::ThreadSleep(200);

    	DevIpTdm::BfLed::Set(0, false);	    	
    	DevIpTdm::BfLed::Set(1, false);	    	    	    	
    }

    // -----------------------------------------

    void RunTestFlash(ExtFlash::IFlashTestIntf &flash)
    {
        class Callback : public MiniFlasher::IFlashTestCallback
        {
            int m_prevPrintPercent;

        // IFlashTestCallback impl
        private:

            void FlashTestState(int percent, int readErrors, int writeErrors)
            {
                const int CPercentStep = 4;
                
                if ((percent - m_prevPrintPercent > CPercentStep) || (percent == 100))
                {
                    std::ostringstream oss;
                    oss << percent << "%";
                    if (readErrors + writeErrors != 0)
                    {
                        oss << "; readErr " << readErrors << "; writeErrors " << writeErrors;
                    }

                    std::cout << oss.str() << std::endl;
                    
                    m_prevPrintPercent = percent;
                }
            }

        public:

            Callback() : m_prevPrintPercent(0)
            {
            }
        };

        Callback c;
        FlashTest test(flash, c, false);
    }
    
    // -----------------------------------------
    
    void PrintFlashDump(Flash &flash, dword addr, int count = 16)
    {
		std::vector<byte> buff;
		buff.resize(count);
		
		flash.Driver().Read(addr, &buff[0], count);
		
		std::cout << Utils::DumpToHexString(&buff[0], count) << std::endl;
    }


    
}  // namespace

// ----------------------------------------

namespace MiniFlasher
{

    void RunServerHost()
    {
        iVDK::VdkInitDone();    	    	
        Utils::DelayInitHost::Inst().DoInit();

        BfDev::SysProperties::InitSysFreq133000kHz();
        
        // StartupLeds();
        DevIpTdm::BfLed::SetColor(DevIpTdm::GREEN);	        	
       	DevIpTdm::BfLed::Set(0, true);	    	

        // zlib        
        // UtilsTests::ZlibRunTest(false);

        try
        {
            bool asyncMode;

            // sync uart
            // asyncMode = false;
            // UartTransport uart;

            // async uart
            asyncMode = true;
            AsyncUartTransport uart;

            Flash flash;
            SysInfra sys;
            
			// debug            
            {
            	const bool CEnableTerminalDebug = false;
            	const bool CRunTestFlash = false;
            	
            	if (CEnableTerminalDebug) UartEcho(uart);  
            	if (CRunTestFlash) RunTestFlash(flash.TestInterface());            	
            	// PrintFlashDump(flash, 0x0000179e);
            }

            ServerProfile profile(uart, flash, sys, asyncMode);

            RunMfServer(profile);
        }
        catch(/* const */ ExtFlash::M25P128Absent &e)
        {
            FlashNotFound();
        }       
        catch(std::exception &e)
        {
        	ESS_HALT(e.what());
        }

    }
    
}  // namespace MiniFlasher

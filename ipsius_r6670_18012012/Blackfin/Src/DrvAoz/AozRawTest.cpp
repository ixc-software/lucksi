#include "stdafx.h"

#include <ccblkfn.h>
#include <cdefBF537.h>

#include "BfDev/SysProperties.h"
#include "bftdm/tdm.h"
#include "Utils/BidirBuffer.h"
#include "Utils/TimerTicks.h"
#include "iVDK/VdkCheckInit.h"
#include "BfDev/BfTimerCounter.h"
#include "DevIpTdm/BfLed.h"
#include "DevIpTdm/BfKeys.h"
#include "ExtFlash/flashmap.h"
#include "Ds28cn01/DS28CN01.h"
#include "ds2155/boardds2155.h"
#include "iUart/IUartIO.h"

#include "Platform/Platform.h"
#include "iUart/IUartIO.h"
#include "BfDev/BfUartSimple.h"
#include "BfTdm/tdm.h"
#include "Utils/MemoryPool.h"
#include "DevIpTdm/DevIpTdmSpiMng.h"
#include "BfDev/BfSpiLoader.h"
#include "ExtFlash/M25P128.h"

#include "DevIpTdm/BfLed.h"

// #include "leds.h"
#include "AozRawTest.h"

// ------------------------------------------------------------

namespace DrvAoz
{

    using namespace Platform;
    using DevIpTdm::BfLed;

    
	enum
    {
        
        CDialToneBlockSize = 8,        
        CAOZCTRLBlockSize = 8,
        
        
        CTDMBlockSize = 160,
        
        
    	CLoadBlockSize = 512,
    	CSlaveBinFlashOffset = 0xc800,
    	CSlaveBinSize = 30000
    	
    };	
    
        
	class AOZConn : public boost::noncopyable,
		BfTdm::ITdmObserver
	{
        BfDev::BfUartSimple m_uart0Obj;
        Utils::TimerTicks	m_timer0Obj;				

        byte	m_buff[CLoadBlockSize];
        int		m_CheckSyncBuff[100];


        word            m_processedBlocks;
        byte 			m_CTRLbuff[CAOZCTRLBlockSize];
        byte 			m_DialBuff[CTDMBlockSize];		

        // state
        byte 			m_signature;
        byte 			m_version;
        byte 			m_hookA;        
        byte 			m_hookB;                
        byte            m_callA;
        byte            m_callB;

        void MakeSync()
        {
            *pTIMER_DISABLE		|= 0x0038;        	    	    
            *pPORTFIO_INEN		|= 0x8000;
            *pPORTF_FER	|= 0x8070;

            *pTIMER4_CONFIG		= 0x00AD;   // 2048 KHz for TimeShift of TDM_CLK
            *pTIMER4_PERIOD		= 8;
            *pTIMER4_WIDTH		= 4;

            *pTIMER3_CONFIG		= 0x00A9;   // !4096 KHz for TimeShift of TDM_CLK
            *pTIMER3_PERIOD		= 4;
            *pTIMER3_WIDTH		= 2;

            *pTIMER5_CONFIG		= 0x00AD;  //AOZ_Frame
            *pTIMER5_PERIOD		= 2048;
            *pTIMER5_WIDTH		= 4;

            *pTIMER_ENABLE		|= 0x0038;        				
        }

        void AOZInitBuffers()
        {
            // control data
            byte CTRLtable[] = {0x1B,0x08,0xB5,0xCA,0xFF,0,0,0};
            for (word i=0; i<CAOZCTRLBlockSize; i++) m_CTRLbuff[i] = CTRLtable[i];  	        

            // sound data
            byte table[] = {0x3a,0xd5,0xba,0xa3,0xba,0xd5,0x3a,0x23};   //1KHz    	        

            byte sin_num=0;
            for (word i=0; i<CTDMBlockSize; i++)
            {
                m_DialBuff[i] = table[sin_num];
                sin_num++;
                if (sin_num>7) sin_num =0;
            }        
        }

        void ShowStatus()
        {
            const char *CLF = "\r\n";
            std::ostringstream log;						
            log<< "Board Signature : 0x" <<std::hex << (int)m_signature << CLF;
            log<< "Board Version : 0x" <<std::hex << (int)m_version << CLF;        
            log<< "Board HookA : 0x" <<std::hex << (int)m_hookA << CLF;                
            log<< "Board HookB : 0x" <<std::hex << (int)m_hookB << CLF;
            m_uart0Obj.SendMsg(log.str().c_str());
        }

        /*
        void UpdateControl(BfTdm::IUserBlockInterface &ch)
        {
            word tssize = ch.GetTSBlockSize();
            byte TSBuff[CTDMBlockSize];

            for(byte ts = 0; ts < 8; ts++)
            {        
                if(ts == 6)
                {
                    for(word i = 0; i < tssize; i++) TSBuff[i] = m_callA;
                    ch.PutTxTSBlockFrom(ts, &TSBuff[0], 0, tssize);
                    for(word i = 0; i < tssize; i++) TSBuff[i] = m_callB;            	
                    ch.PutTxTSBlockFrom(ts+8, &TSBuff[0], 0, tssize);
                }
                else
                {
                    for(word i = 0; i < tssize; i++) TSBuff[i] = m_CTRLbuff[ts];
                    ch.PutTxTSBlockFrom(ts, &TSBuff[0], 0, tssize);
                    ch.PutTxTSBlockFrom(ts+8, &TSBuff[0], 0, tssize);
                }
            }
        } */

        /*
        void UpdateControl(BfTdm::IUserBlockInterface &ch)
        {
            byte tsMap[] = {0x1B, 0x08, 0xB5, 0xCA, 0xFF, 0, 0, 0, 
                               0,    0,    0,    0,    0, 0, 0, 0};

            int blockSize = ch.GetTSBlockSize();
            byte buff[CTDMBlockSize];

            ESS_ASSERT(blockSize == CTDMBlockSize);

            // patch m_call to tsMap
            // ... 

            for(int i = 0; i < sizeof(tsMap); ++i)
            {
                std::memset(buff, tsMap[i], sizeof(buff));
                ch.PutTxTSBlockFrom(i, buff, 0, sizeof(buff));
            }
            
        } */

        void UpdateControl(BfTdm::IUserBlockInterface &ch)
        {
            byte tsMap[] = {0x00, 0x00, 0x35, 0xCA, 0x00, 0, 0, 0, 
                0,    0,    0,    0,    0, 0, 0, 0};

            int blockSize = ch.GetTSBlockSize();
            
            tsMap[6] = m_callA;
            tsMap[14] = m_callB;
            
            for(int frame = 0; frame < blockSize; ++frame)
            {
                word *pFrame = ch.GetTxBlock(frame);

                for(int i = 0; i < sizeof(tsMap); ++i)
                {
                    pFrame[i] = tsMap[i];
                }
            }

        }



    // ITdmObserver impl
    private:

        void NewBlockProcess(
            word sportNum,
            BfTdm::IUserBlockInterface &ch0,
            BfTdm::IUserBlockInterface &ch1,
            bool collision)
        {
        	ESS_ASSERT(!collision);
        	
            // update state
            ch1.CopyRxTSBlockTo(0, &m_signature, 0, 1);
            ch1.CopyRxTSBlockTo(3, &m_version, 0, 1);       	
            ch1.CopyRxTSBlockTo(1, &m_hookA, 0, 1);       	
            ch1.CopyRxTSBlockTo(9, &m_hookB, 0, 1);       	

            // set call
            if(m_hookA != 0xff || m_hookB != 0xff)
            {
                m_callA = 0;
                m_callB = 0;            
            }

            // put control
            UpdateControl(ch1);

            // done
            ch0.UserFree();
            ch1.UserFree();        

            m_processedBlocks++;
        }
        

	public:

        AOZConn() :
              m_uart0Obj(BfDev::SysProperties::Instance().getFrequencySys(), 0, 115200),
              m_processedBlocks(0),
              m_callA(0),
              m_callB(0)
          {         
              MakeSync();
              AOZInitBuffers();
          }

        void Process()
        {
            m_uart0Obj.SendMsg("AOZ Debug Project \n");

            BfTdm::Tdm  TdmObj( *this, 0, BfTdm::TdmProfile(3, CTDMBlockSize, 1, BfTdm::CNoCompanding));		

            TdmObj.Enable();        		
            int keys = 0;		

            while (true)
            {                               	
                iVDK::Sleep(5);  

                BfLed::SetColor( (m_signature == 0xa6) ? DevIpTdm::GREEN : DevIpTdm::RED );	

                keys++;
                if(keys >= 100)
                {
                    keys = 0;
                    DevIpTdm::BfLed::Set(1, DevIpTdm::BfKeys::Get(DevIpTdm::KEY1));
                    //				if (DevIpTdm::BfKeys::Get(0)) SecondBFLoadSpi();
                    if (DevIpTdm::BfKeys::Get(0)) 
                    {
                        ShowStatus();
                        if(m_hookA == 0xff && m_hookB == 0xff)
                        {
                            m_callA = 0xff;
                            m_callB = 0xff;				        
                        }
                    }
                    else
                    {
                        m_callA = 0;
                        m_callB = 0;				        				        
                    }			
                }	        
            }                

        }
                  
	};


}  // namespace DrvAoz


// ------------------------------------------------------------

namespace DrvAoz
{    

    void RunRawAoz()
    {
    	/*
        iVDK::VdkInitDone();    	    	
        BfDev::SysProperties::InitSysFreq133000kHz(); */

        try
        {
            AOZConn AOZObj;    
            AOZObj.Process();
        }
        catch(std::exception &e)
        {
            while(true)
            {
                BfLed::SetColor(DevIpTdm::RED);	
                iVDK::Sleep(300);
                BfLed::SetColor(DevIpTdm::OFF);	        	
                iVDK::Sleep(300);	        	
            }
        }                                                      

    }

}

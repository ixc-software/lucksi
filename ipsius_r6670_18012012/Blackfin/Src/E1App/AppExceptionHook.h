#ifndef __APPEXCEPTIONHOOK__
#define __APPEXCEPTIONHOOK__

#include "Utils/ErrorsSubsystem.h"
#include "BfDev/BfUartSimpleCore.h" 
#include "BfDev/SysProperties.h"
#include "DevIpTdm/BfLed.h"
#include "iVDK/CriticalRegion.h"

namespace E1App
{

    class AppExceptionHook : public ESS::ExceptionHook
    {
        volatile int m_counter;
        Platform::dword m_sysFreq;
        boost::scoped_ptr<BfDev::BfUartSimpleCore> m_uart;

        void HaltCycle(const char *pMsg) const  // where is no allocation
        {
            const int CPeriodMs = 500;

            // print to cout
            std::cout << pMsg << std::endl;

            // loop forever
            iVDK::CriticalRegion::Enter();

            BfDev::BfTimerCounter t(0, m_sysFreq, true);
            int interval = t.NsToCounter(CPeriodMs * 1000 * 1000);

            int color = DevIpTdm::OFF;
            while (true)
            {    
                DevIpTdm::BfLed::SetColor(color);
                color = (color == DevIpTdm::OFF) ? DevIpTdm::RED : DevIpTdm::OFF;
                t.Sleep(interval);

                if (m_uart) 
                {
                    m_uart->Send(pMsg);
                    m_uart->Send("\n\r");
                }
            }
        }

        void Hook(const ESS::BaseException *pE)  // override
        {
            ++m_counter;  // for breakpoint 

            if (dynamic_cast<const ESS::Assertion*>(pE))
            {
                HaltCycle( pE->what() );
            }
        }

    public:

        AppExceptionHook(bool uartEnable)
        {
            m_sysFreq = BfDev::SysProperties::Instance().getFrequencySys();

            if (uartEnable)
            {
                m_uart.reset( new BfDev::BfUartSimpleCore(m_sysFreq, 0, 115200) );
            }
        }

    };
        
}  // namespace E1App

#endif

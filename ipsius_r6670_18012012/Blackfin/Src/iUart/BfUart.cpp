#include "stdafx.h"

#include <drivers/adi_dev.h>
#include <drivers/uart/adi_uart.h>
#include "adi_ssl_init.h"
#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IntSet.h"
#include "BfDev/AdiDevice.h"
#include "BfUartBuffers.h"
#include "BfUartProfile.h"
#include "BfUartManagement.h"
#include "BfUartIO.h"
#include "BfUart.h"


namespace 
{ 
	
    using namespace iUart;
    
    
    class UartList : boost::noncopyable
    {
    public:
    	enum{
			CCountUartPort = 2
    	};
    	
		UartList() 
		{
			for(int i = 0; i < Size(); ++i)
			{
				m_arr[i] = 0;
			}
		}
    	~UartList()
    	{
    		for(int i = 0; i < Size(); ++i)
    		{
	    		ESS_ASSERT(m_arr[i] == 0);
    		}
    	}
    	
    	bool IsExist(const BfUart *uart) const
    	{
    		for(int i =0; i < Size(); ++i)
    		{
    			if(m_arr[i] == uart) return true;
    		}
			return false;
    	}

		void Add(int port, BfUart *uart)
    	{
    		ESS_ASSERT(uart != 0);
    		ESS_ASSERT(port < Size());
    		ESS_ASSERT(m_arr[port] == 0);
    		
    		m_arr[port] = uart;	
    	}
    	
    	void Delete(const BfUart *uart)
    	{
    		for(int i =0; i < Size(); ++i)
    		{
    			if(m_arr[i] == uart)
    			{
    				m_arr[i] = 0;
    				return;
    			}
    		}
    		ESS_HALT("Unknown Uart");
    	}
		
    	BfUart *GetUart(int port)
    	{
    		ESS_ASSERT(port < Size());
    		return m_arr[port];
/*    		    		
    		BfUart *uart = m_arr[port];
    		ESS_ASSERT(uart != 0);
    		return *uart;*/
    	}    	
    private:
    	int Size() const 
    	{
    		return CCountUartPort;
    	}    	
    private:
    	BfUart *m_arr[CCountUartPort];
    };
	UartList GlobalUartList;
	
};


namespace iUart
{
	using Platform::byte;

    class BfUart::Impl : boost::noncopyable
    {
		struct ProfileValidator
		{
			ProfileValidator(const BfUartProfile &profile)
	        {
	            profile.Validation();
        		//ESS_ASSERT(profile.m_port==0 || profile.m_port==1);
	        }
		};
		ProfileValidator m_profileValidator;
    	

        BfDev::AdiDevice m_device;
        BfUartManagement m_management;
        BfUartIO m_io;
    private:        
        BfDev::AdiDeviceSettings 
        	Settings(BfUart &owner, const BfUartProfile &profile)
		{
			BfDev::AdiDeviceSettings settings;
			settings.m_pEntryPoint = (profile.m_useDma) ? 
				&ADIUARTDmaEntryPoint : &ADIUARTEntryPoint;
			settings.m_clientHandle = &owner;
    		settings.m_direction = ADI_DEV_DIRECTION_BIDIRECTIONAL;
			settings.m_DMAHandle = (profile.m_useDma) ? adi_dma_ManagerHandle : 0;
			settings.m_DCBHandle = 0;
			settings.m_clientCallback = GlobalCallback;
			return settings;
		}
    private:
        
        static void GlobalCallback(void *pHandle, u32 event, void *pArg)
        {
        	
        	BfUart *uart = static_cast<BfUart *>(pHandle);
        	ESS_ASSERT(GlobalUartList.IsExist(uart));	
        	
            switch (event) 
            {
            case ADI_DEV_EVENT_BUFFER_PROCESSED:
                (uart->m_impl->m_io).BufferProcessed(static_cast<UartBufferItem*>(pArg));
                break;
            case ADI_UART_EVENT_AUTOBAUD_COMPLETE:
//                if(m_user) m_user->ReadyToWork(&m_wrapper);				
                break; 
            case ADI_UART_EVENT_BREAK_INTERRUPT:
            case ADI_UART_EVENT_FRAMING_ERROR:
            case ADI_UART_EVENT_PARITY_ERROR:
            case ADI_UART_EVENT_OVERRUN_ERROR:
				(uart->m_impl->m_io).ErrorInInterrupt(event);
				break; 
            default:            
                ESS_HALT("Unknown event");
                break; 
            }
        }    
    
    public:	
    
        Impl(BfUart &owner,
        	BfDev::AdiDeviceManager &deviceManager,
            const BfUartProfile &profile, 
			ILogForUart *log) :
			m_profileValidator(profile),
            m_device(profile.m_port, deviceManager, Settings(owner, profile)),
            m_management(m_device, profile),
            m_io(m_device, profile, log)
        {}

        void Start()
        {
        	{
				UartReturnCode result = m_management.DataflowChained();
				if(!result.isSuccess()) return;			
        	}
        	
			m_io.SafeStartReading();
			
			{			
				UartReturnCode result = m_management.Dataflow(true);
				if(!result.isSuccess()) return;
			}
        }
                
        IBfUartManagement &Management()    {   return m_management;    }
        iUart::IUartIO &IO()    {   return m_io;    }
        const iUart::IUartIO &IO() const {   return m_io;    }
    };

   	
    //----------------------------------------------------------------
       	    
	BfUart::BfUart(BfDev::AdiDeviceManager &deviceManager,
        const BfUartProfile &profile, 
		ILogForUart *log) :
    	m_impl(
   			new Impl(*this, deviceManager, profile, log))// can throw
    {
		GlobalUartList.Add(profile.Port(), this);
		m_impl->Start();
    }
    
    //----------------------------------------------------------------

    BfUart::~BfUart()
    {
		GlobalUartList.Delete(this);    
    }
    
    //----------------------------------------------------------------

    IBfUartManagement &BfUart::Management()
    {
        return m_impl->Management();
    }

    //----------------------------------------------------------------

    iUart::IUartIO &BfUart::IO()
    {
        return m_impl->IO();
    }

    //----------------------------------------------------------------

    const iUart::IUartIO &BfUart::IO() const
    {
        return m_impl->IO();
    }
    
	BfUart *BfUart::GetBfUart(int port)
	{
		return GlobalUartList.GetUart(port);
	}
	
} // namespace BfDev


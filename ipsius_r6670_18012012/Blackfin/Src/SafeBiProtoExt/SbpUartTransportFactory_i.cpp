#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/SafeRef.h"
#include "BfDev/AdiDeviceManager.h"
#include "iUart/BfUart.h"
#include "iUart/BfUartProfile.h"
#include "SbpUartTransport.h"
#include "SbpUartTransportFactory.h"

namespace 
{
    using namespace Platform;
    using namespace SBProto;    

    class UartTransport : 
		public SBProtoExt::SbpUartTransport,
        public virtual Utils::SafeRefServer,
        public iUart::IBfUartEvents
    {
    public:
        UartTransport(size_t buffCapacity, const iUart::BfUartProfile &profile, size_t maxSendSize) :
        	SBProtoExt::SbpUartTransport(maxSendSize, buffCapacity),
			m_deviceManager(1),
			m_uart(m_deviceManager, profile)
        {
			SbpUartTransport::InitUart(&m_uart.IO());
        }

    // BfDev::IBfUartEvents
    private:
		void ReadyToWork(const iUart::BfUart *src)
		{
			ESS_ASSERT(&m_uart == src);
		}

        void DataReceived(const iUart::BfUart *src)
		{
			ESS_ASSERT(&m_uart == src);
		}


        void DataSent(const iUart::BfUart *src)
		{
			ESS_ASSERT(&m_uart == src);
		}


        void Error(const iUart::BfUart *src, iUart::UartReturnCode error)
        {
			ESS_ASSERT(&m_uart == src);
        }
    private:
		BfDev::AdiDeviceManager m_deviceManager;
        iUart::BfUart m_uart;
    };
};

// ------------------------------------------------------------

namespace SBProtoExt
{


	boost::shared_ptr<SBProto::ISbpTransport>
 		SbpUartTransportFactory::CreateTransport(int comPort, const SbpUartProfile &profile, std::string &error, bool isServerInd, size_t maxSendSize)
	{
	    iUart::BfUartProfile uartProfile(
	        comPort, 
			256, 
			512,
			profile.BoudRate,
			profile.DataBits,
			profile.StopBits);
			
		
		uartProfile.m_parityType = profile.ParityType;

		boost::shared_ptr<SBProto::ISbpTransport> transport;
		try
		{
			transport.reset(new UartTransport(profile.BuffCapacity, uartProfile, maxSendSize));
		}
		catch (iUart::IBfUartManagement::InitializationFailed &e)
		{
			error = e.getTextMessage();
		}
		return transport; 
	}
    
};





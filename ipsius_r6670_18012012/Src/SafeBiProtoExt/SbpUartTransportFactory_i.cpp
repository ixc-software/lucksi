#include "stdafx.h"
#include "iUart/Uart.h"
#include "SbpUartTransport.h"
#include "SbpUartTransportFactory.h"

namespace 
{
	using namespace Platform;
	using namespace SBProto;
	using namespace SBProtoExt;

	class UartTransport : 
		public SBProtoExt::SbpUartTransport,
		public virtual Utils::SafeRefServer,
		public iUart::IUartEvents
	{
	public:
		UartTransport(const iUart::UartProfile &profile, size_t maxSendSize = 0) 
            : SBProtoExt::SbpUartTransport(maxSendSize),
            m_uart(profile)
		{
			m_uart.BindUser(this);
			SbpUartTransport::InitUart(&m_uart.IO());
		}
		~UartTransport()
		{
			m_uart.UnbindUser(this);
		}

	// Uart::IUartEvents
	private:
		void Error(const iUart::Uart *src, iUart::UartReturnCode error)
		{
			ESS_ASSERT(&m_uart == src);
		}
	private:
		iUart::Uart m_uart;
	};
};

namespace SBProtoExt
{
	boost::shared_ptr<SBProto::ISbpTransport>
		SbpUartTransportFactory::CreateTransport(int comPort, const SbpUartProfile &profile, std::string &error, bool isServerInd, size_t maxSendSize)
	{
		if (isServerInd) return boost::shared_ptr<SBProto::ISbpTransport>();

		iUart::UartProfile uartProfile(comPort,
			profile.BoudRate,
			profile.DataBits, profile.StopBits, 
			profile.ParityType, iUart::FlowOff);	
		boost::shared_ptr<SBProto::ISbpTransport> transport;
		try
		{
			transport.reset(new UartTransport(uartProfile, maxSendSize));
		}
		catch (iUart::Uart::InitializationFailed &e)
		{
            error = "Can`t create UartTransport because: " + e.getTextMessage();
		}
		return transport;
	}
};





#ifndef _SBP_UART_TRANSPORT_FACTORY_H_
#define _SBP_UART_TRANSPORT_FACTORY_H_


#include "iUart/UartConst.h"
#include "SafeBiProto/ISbpTransport.h"

namespace SBProtoExt
{
	struct SbpUartProfile 
	{
		SbpUartProfile()
		{
			BoudRate = 38400;
			DataBits = iUart::DataBits_8;
			StopBits = iUart::StopBits_1;
			ParityType = iUart::ParityNone;
            BuffCapacity = 0; // avto
		}
		bool IsValidation() const;
		int  BoudRate;
		iUart::DataBits  DataBits;
		iUart::StopBits  StopBits;
		iUart::ParityType ParityType;
        size_t BuffCapacity;
	};

	class SbpUartTransportFactory : boost::noncopyable
    {
    public:
		static boost::shared_ptr<SBProto::ISbpTransport>
			CreateTransport(int comPort, const SbpUartProfile &profile, std::string &error, bool isServerInd, size_t maxSendSize = 0);
    };
};


#endif


#ifndef _BF_UART_DRIVER_H_
#define _BF_UART_DRIVER_H_

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "IBfUart.h"
#include "BfUartProfile.h"

namespace iLogW {	class ILogSessionCreator; };
namespace BfDev 
{	
	class AdiDeviceManager; 
};

namespace iUart
{
	class IBfUartEvents;
	class IBfUartManagement;
		
	class ILogForUart : public Utils::IBasicInterface
	{	
	public:
		virtual void Log(const std::string &msg) = 0;
	};

	class BfUart : boost::noncopyable
    {
    public:
        BfUart(BfDev::AdiDeviceManager &deviceManager,
            const BfUartProfile &profile, 
			ILogForUart *log = 0); // can throw
        	
        ~BfUart();
	    IBfUartManagement &Management();
	    iUart::IUartIO &IO();
	    const iUart::IUartIO &IO() const;
	    static BfUart *GetBfUart(int port);
    private:
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
    };
} // namespace BfDev

#endif

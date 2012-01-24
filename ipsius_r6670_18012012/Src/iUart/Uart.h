#ifndef _MAIN_UART_H_
#define _MAIN_UART_H_

#include "Utils/SafeRef.h"
#include "IUartIO.h"
#include "IUartManagement.h"
#include "UartProfile.h"
#include "IUartEvents.h"

namespace iUart
{
    class Uart : public boost::noncopyable
    {
    public:
		ESS_TYPEDEF(InitializationFailed);
        Uart(const UartProfile &profile, 
            Utils::SafeRef<IUartEvents> user = Utils::SafeRef<IUartEvents>()); // can throw
        ~Uart();
		
		static Uart *CreateUart(const UartProfile &profile, std::string &error)
		{
			return CreateUart(profile, Utils::SafeRef<IUartEvents>(), error);
		}
		static Uart *CreateUart(const UartProfile &profile, 
                                Utils::SafeRef<IUartEvents> user, 
                                std::string &error);
        
        void BindUser(Utils::SafeRef<IUartEvents> user);
		void UnbindUser(const IUartEvents *user);
        IUartIO &IO();
        const IUartIO &IO() const;
        IUartManagement &Management();
        const IUartManagement &Management() const;
    private:
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
    };
};

#endif

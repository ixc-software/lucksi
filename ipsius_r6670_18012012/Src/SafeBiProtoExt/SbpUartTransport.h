#ifndef _SBP_UART_TRANSPORT_H_
#define _SBP_UART_TRANSPORT_H_

#include "Platform/Platform.h"
#include "iUart/IUartIO.h"
#include "SafeBiProto/ISbpTransport.h"

namespace SBProtoExt
{
	class SbpUartTransport : boost::noncopyable,
		public SBProto::ISbpTransport
	{
	public:
		SbpUartTransport(size_t maxSendSize, size_t buffCapacity = 0, iUart::IUartIO *io = 0);
		~SbpUartTransport();
		void InitUart(iUart::IUartIO *io);
	// SBProto::ISbpTransport
	private:
        size_t MaxSendSize()const;
		void BindProto(SBProto::ISbpTransportToProto *proto);
		void UnbindProto();
		void Send(const void *data, size_t size);
		void Connect();
		void Disconnect();
		void BindUser(SBProto::ISbpTransportToUser *user);
		std::string Info() const;
		void Process();
	private:
		bool m_ativatedbyUser;
		iUart::IUartIO *m_io;
		SBProto::ISbpTransportToProto *m_proto;
		SBProto::ISbpTransportToUser *m_user;

        size_t m_maxSendSize;
		std::vector<Platform::byte> m_buff;
	};




};


#endif


#include "stdafx.h"
#include "Utils/RawCRC32.h"
#include "iUart/IUartIO.h"
#include "SbpUartTransport.h"

namespace SBProtoExt
{
	SbpUartTransport::SbpUartTransport(size_t maxSendSize, size_t buffCapacity, iUart::IUartIO *io) : 
		m_ativatedbyUser(false),
		m_io(io), m_proto(0), m_user(0),
        m_maxSendSize(maxSendSize)
	{
		if (buffCapacity > 0) m_buff.reserve(buffCapacity);
	}

	// ------------------------------------------------------------

	SbpUartTransport::~SbpUartTransport()
	{
		ESS_ASSERT(m_proto == 0);
	}

	// ------------------------------------------------------------

	void SbpUartTransport::InitUart(iUart::IUartIO *io)
	{
		ESS_ASSERT(m_io == 0);
		m_io = io;
	}

	// ------------------------------------------------------------
	// SBProto::ISbpTransport

	void SbpUartTransport::BindProto(SBProto::ISbpTransportToProto *proto)
	{
		ESS_ASSERT(!m_proto && proto);

		m_proto = proto;
	}

	// ------------------------------------------------------------

	void SbpUartTransport::UnbindProto()
	{
		m_proto = 0;
	}

	// ------------------------------------------------------------

	void SbpUartTransport::Send(const void *data, size_t size)
	{
		ESS_ASSERT(m_io != 0);
		ESS_ASSERT(m_ativatedbyUser);
        
		try
		{
			m_io->Write(data, size);
		}
		catch (iUart::IUartIO::UartIoWriteError *e)
		{
			m_user->TransportError(e->getTextAndPlace());
		}

	}

	// ------------------------------------------------------------

	void SbpUartTransport::Connect()
	{
		// to do nothing 
		ESS_ASSERT(m_user != 0);

		ESS_ASSERT(!m_ativatedbyUser);
		
		m_ativatedbyUser = true;

		m_user->TransportConnected();
	}

	// ------------------------------------------------------------

	void SbpUartTransport::Disconnect()
	{
		// to do nothing 
		ESS_ASSERT(m_user != 0);
		
		ESS_ASSERT(m_ativatedbyUser);
		
		m_ativatedbyUser = false;
	}

	// ------------------------------------------------------------

	void SbpUartTransport::BindUser(SBProto::ISbpTransportToUser *user)
	{
		ESS_ASSERT(m_user == 0);

		m_user = user;
	}

	// ------------------------------------------------------------

	std::string SbpUartTransport::Info() const
	{
		return "Uart";
	}

	// ------------------------------------------------------------

	void SbpUartTransport::Process()
	{
		ESS_ASSERT(m_io != 0);

		try
		{
			int size = m_io->AvailableForRead();
			if (size == 0) return;
						
			m_buff.resize(size);
			
			size = m_io->Read(&m_buff.at(0), m_buff.size());
			if (size != 0 && m_ativatedbyUser) m_proto->DataReceived(&m_buff.at(0), size);
		}
		catch (iUart::IUartIO::UartIoReadError &e)
		{
			m_user->TransportError(e.getTextAndPlace());
		}


	}

    size_t SbpUartTransport::MaxSendSize() const
    {
        return m_maxSendSize;

    }

};



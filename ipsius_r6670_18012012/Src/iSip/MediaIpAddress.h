#pragma once

#include "boost/asio.hpp"
#include "SipTransportInfo.h"

namespace iSip
{
	class MediaIpAddress
	{
	public:

		explicit MediaIpAddress()
		{
			m_transport = SipTransportInfo::UnknownTransport;
			m_port = 0;
		}

		explicit MediaIpAddress(SipTransportInfo::Type transport, 
			const boost::asio::ip::address& address, unsigned int port) :
			m_transport(transport),
			m_address(address),
			m_port(port)
		{}

		SipTransportInfo::Type getTransportType() const { return m_transport; }

		const boost::asio::ip::address& getAddress() const { return m_address; }

		unsigned int getPort() const { return m_port; }

		std::string ToString() const
		{
			std::ostringstream out;

			out << "{"
				<< SipTransportInfo::ToString(m_transport)
				<< m_address.to_string() 
				<< ":" 
				<< m_port 
				<< "]";

			return out.str();
		}

	private:
		SipTransportInfo::Type m_transport;
		boost::asio::ip::address m_address;
		unsigned int m_port;

	};
}


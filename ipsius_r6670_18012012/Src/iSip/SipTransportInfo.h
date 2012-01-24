#pragma once

#include "Utils/ErrorsSubsystem.h"
#include "Utils/QtEnumResolver.h"

#include <rutil/TransportType.hxx>

namespace iSip
{

	class SipTransportInfo : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:

		// transport type 

		enum Type
		{
			UnknownTransport,
			Udp,
			Tcp,
//			Tls
		};

		Q_ENUMS(Type);

		static Type ConvertFromResip(resip::TransportType resipType)
		{
			if (resipType ==  resip::UDP) return Udp;

			if (resipType ==  resip::TCP) return Tcp;

			ESS_HALT ("Wrong value");

			return UnknownTransport;
		}

		static resip::TransportType ConvertToResip(Type type)
		{
			if (type ==  Udp) return resip::UDP;

			if (type ==  Tcp) return resip::TCP;

			return resip::UNKNOWN_TRANSPORT;
		}


		// IpVersion

		enum IpVersion
		{
			UnknownIpVersion,
			Ip4,
			Ip6
		};

		Q_ENUMS(IpVersion);

		static IpVersion ConvertFromResip(resip::IpVersion resipIp)
		{
			if (resipIp ==  resip::V4) return Ip4;

			if (resipIp ==  resip::V6) return Ip6;

			ESS_HALT ("Wrong value");

			return Ip4;
		}

		static resip::IpVersion ConvertToResip(IpVersion ip)
		{
			if (ip == Ip4) return resip::V4;

			if (ip == Ip6) return resip::V6;

			ESS_HALT ("Wrong value");

			return resip::V4;
		}

		template <class Value> static std::string ToString(Value value)
		{
			return Utils::QtEnumResolver<SipTransportInfo, Value>().
				Resolve(value);
		}

	};
	
}
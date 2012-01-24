#include "stdafx.h"

#include "SipUtils.h"
#include "iSip/undeferrno.h"
#include "SipTrustedNode.h"
#include "Utils/ErrorsSubsystem.h"

#include "rutil/ParseBuffer.hxx"
#include "rutil/DnsUtil.hxx"
#include "resip/stack/Tuple.hxx"

namespace
{

	bool IsDtlsSupported()	{	return false;	}

	bool IsSslSupported()	{	return false;	}
		
	bool IsIpV6Supported()	{	return true;	}

	bool IsIpV6Address(const resip::Data &address, QString &error)
	{
		if (!IsIpV6Supported())
		{	
			error = "Ip v6 isn't supported";
			return false;
		}

		// try to convert into IPv6 network form
		if (resip::DnsUtil::isIpV6Address(address)) return true;

		error = "Ip v6 parsing error";
		return false;
	}

	bool IsIpV4Address(const resip::Data &address)
	{
		return resip::DnsUtil::isIpV4Address(address);
	}

	bool CheckMask (iSip::SipTransportInfo::IpVersion ipVersion, int mask, QString &error)
	{
		if (ipVersion = iSip::SipTransportInfo::Ip4)
		{
			if (mask < 8 || mask > 32)
			{
				error = "Wrong mask for ipv4. Mask should be: 8 <= mask <= 32";
				return false;
			}
		}
		else if (ipVersion = iSip::SipTransportInfo::Ip6)
		{
			if (mask < 64 || mask > 128)
			{
				error = "Wrong mask for ipv6. Mask should be: 64 <= mask <= 128";
				return false;
			}
		}
		else ESS_HALT ("Unknow ip version");

		return true;
	}
}


namespace iSip
{
	class SipNodeList::Node
	{
	public:

		Node (resip::Tuple addressTuple,
			int	mask) :
			m_addressTuple(addressTuple),
			m_mask(mask)
		{}

		// -------------------------------------------------------------------------------------

		bool operator == (const Node &val) const
		{	
			return (this == &val) ?
				true : false;
					(m_addressTuple == val.m_addressTuple && m_mask == val.m_mask);
		}

		// -------------------------------------------------------------------------------------

		bool IsInThatNode(const resip::Tuple& address) const
		{
			return
				m_addressTuple.isEqualWithMask(address, m_mask, m_addressTuple.getPort() == 0);
		}

		// -------------------------------------------------------------------------------------

		QString Subnet() const
		{
			return iSip::SipUtils::ToQString(m_addressTuple.presentationFormat()) +
				"/" + QString().setNum(m_mask);
		}

		// -------------------------------------------------------------------------------------

		int Port() const	{	return m_addressTuple.getPort();	}


	private:

		resip::Tuple m_addressTuple;
		int	m_mask;

	};
}	


namespace iSip
{
	SipNodeList::SipNodeList() :
		m_list(new List())
	{

	}
	
	// -------------------------------------------------------------------------------------
	
	SipNodeList::~SipNodeList()
	{

	}

	// -------------------------------------------------------------------------------------

	bool SipNodeList::AddNode(const QString &val,
		int port,
		SipTransportInfo::Type transport,
		QString &error)
	{
		// Input can be in any of these formats
		// localhost         localhost  (becomes 127.0.0.1/8, ::1/128 and fe80::1/64)
		// bare hostname     server1
		// FQDN              server1.example.com
		// IPv4 address      192.168.1.100
		// IPv4 + mask       192.168.1.0/24
		// IPv6 address      :341:0:23:4bb:0011:2435:abcd
		// IPv6 + mask       :341:0:23:4bb:0011:2435:abcd/80
		// IPv6 reference    [:341:0:23:4bb:0011:2435:abcd]
		// IPv6 ref + mask   [:341:0:23:4bb:0011:2435:abcd]/64

		if (val.isEmpty())
		{
			error = "Wrong value. Parameter is empty";
			return false;
		}

		try
		{
			resip::ParseBuffer pb(SipUtils::ToUtf8(val));
			resip::Data hostOrIp;
			const char* anchor = pb.start();
			SipTransportInfo::IpVersion ipVersion = SipTransportInfo::UnknownIpVersion;

			int mask;

			if (*pb.position() == '[')   // encountered beginning of IPv6 reference
			{
				anchor = pb.skipChar();
				pb.skipToEndQuote(']');

				pb.data(hostOrIp, anchor);  // copy the presentation form of the IPv6 address
				anchor = pb.skipChar();

				// try to convert into IPv6 network form

				if (!IsIpV6Address(hostOrIp, error))return false;

				ipVersion = SipTransportInfo::Ip4;
			}
			else
			{
				pb.skipToOneOf(".:");

				if (pb.position() == pb.end())   // We probably have a bare hostname
				{
					pb.data(hostOrIp, anchor);
					if (hostOrIp.lowercase() == "localhost") return AddHostNode(hostOrIp, error);

					// add special localhost addresses for v4 and v6 to list and return
					AddIpNode("127.0.0.1", 8, port, SipTransportInfo::Ip4, transport);
					AddIpNode("::1", 128, port, SipTransportInfo::Ip6, transport);
					AddIpNode("fe80::1", 64, port, SipTransportInfo::Ip6, transport);
					return true;
				}

				if (*pb.position() == ':')     // Must be an IPv6 address
				{
					pb.skipToChar('/');
					pb.data(hostOrIp, anchor);  // copy the presentation form of the IPv6 address

					if (!IsIpV6Address(hostOrIp.c_str(), error))return false;

					ipVersion = SipTransportInfo::Ip6;
				}
				else // *pb.position() == '.'
				{
					// Could be either an IPv4 address or an FQDN
					pb.skipToChar('/');
					pb.data(hostOrIp, anchor);  // copy the presentation form of the address

					// try to interpret as an IPv4 address, if that fails look it up in DNS
					if (!IsIpV4Address(hostOrIp.c_str()))
					{
						// hopefully it is a legal FQDN, try it.
						return AddHostNode(hostOrIp, error);
					}

					// it was an IPv4 address
					ipVersion = SipTransportInfo::Ip4;
				}
			}

			ESS_ASSERT (ipVersion != SipTransportInfo::UnknownIpVersion);

			if (pb.eof() || *pb.position() != '/')
			{
				// ipv4 -> mask = 32; ipv6 -> mask = 128;
				mask = (ipVersion == SipTransportInfo::Ip4) ? 32 : 128;
			}
			else
			{
				pb.skipChar();
				mask = pb.integer();
				if (!CheckMask(ipVersion, mask, error)) return false;
			}	

			if (!pb.eof())
			{
				error = "Wrong mask value";
				return false;
			}

			AddIpNode(hostOrIp, mask, port, ipVersion, transport);
			return true;

		}
		catch(resip::ParseException &e)
		{
			error = SipUtils::ToQString(e);
		}
		return false;
	}

	// -------------------------------------------------------------------------------------

	void SipNodeList::AddIpNode(const resip::Data &ip,
		int mask,
		int port,
		SipTransportInfo::IpVersion ipVersion,
		SipTransportInfo::Type transport)
	{
		Node node(
			resip::Tuple(ip,
				port,
				SipTransportInfo::ConvertToResip(ipVersion),
				SipTransportInfo::ConvertToResip(transport)),
			mask);
		
		if (m_list->end() != std::find(m_list->begin(), m_list->end(), node)) return;

		m_list->push_back(node);
	}

	// -------------------------------------------------------------------------------------

	bool SipNodeList::AddHostNode(const resip::Data &host, QString &error)
	{
		if (!IsDtlsSupported() && !IsSslSupported())
		{
			error = "Dtls and Ssl aren't supported";
			return false;
		}

		ESS_UNIMPLEMENTED;
		return false;
	}

	// -------------------------------------------------------------------------------------

	void SipNodeList::DeleteNode(int index)
	{
		ESS_ASSERT (index < m_list->size());

		List::iterator i = m_list->begin();
		std::advance(i, index);

		m_list->erase(i);
	}

	// -------------------------------------------------------------------------------------

	void SipNodeList::ClearAll()	{	m_list.reset(new List());	}

	// -------------------------------------------------------------------------------------

	QString SipNodeList::Subnet(int index) const{	return m_list->at(index).Subnet(); }

	// -------------------------------------------------------------------------------------

	int SipNodeList::Port(int index) const {	return m_list->at(index).Port(); }

	// -------------------------------------------------------------------------------------
		
	bool SipNodeList::IsItInList(const QHostAddress &address, int port,SipTransportInfo::Type transport) const
	{
		ESS_ASSERT (address.protocol() == QAbstractSocket::IPv4Protocol ||
			address.protocol() == QAbstractSocket::IPv6Protocol);

		SipTransportInfo::IpVersion ipVersion = (address.protocol() == QAbstractSocket::IPv4Protocol) ?
			SipTransportInfo::Ip4 :
			SipTransportInfo::Ip6;

		resip::Tuple tuple(SipUtils::ToUtf8(address.toString()),
			port,
			SipTransportInfo::ConvertToResip(ipVersion),
			SipTransportInfo::ConvertToResip(transport));

		return IsItInList(tuple);
	}

	// -------------------------------------------------------------------------------------

	bool SipNodeList::IsItInList(const resip::Tuple &address) const
	{
		for (List::const_iterator i = m_list->begin(); i != m_list->end(); ++i )
		{
			if (i->IsInThatNode(address)) return true;
		}
		return false;
	}

	// -------------------------------------------------------------------------------------

	bool SipNodeList::Empty() const {	return m_list->empty(); }

	// -------------------------------------------------------------------------------------

	int  SipNodeList::Size() const	{	return m_list->size(); }

	// -------------------------------------------------------------------------------------
	// SipTrustedNodeList

	bool SipTrustedNodeList::IsTrustedNode(const resip::Tuple &address) const
	{
		if (BanList().IsItInList(address)) return false;

		if (TrustedList().Empty()) return true;

		return TrustedList().IsItInList(address);
	}

}
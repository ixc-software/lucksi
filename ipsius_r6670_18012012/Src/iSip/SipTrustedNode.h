#pragma once

#include "SipTransportInfo.h"
#include "Utils/ErrorsSubsystem.h"
#include "rutil/TransportType.hxx"

namespace resip {	class Tuple; }

namespace iSip
{
	// ---------------------------------------------------------------

	class SipNodeList : boost::noncopyable
	{
	public:

		SipNodeList();
		~SipNodeList();

		bool AddNode(const QString &val, 
			int port, 
			SipTransportInfo::Type transport, 
			QString &error);

		void DeleteNode(int index);

		void ClearAll();

		bool IsItInList(const QHostAddress &address, int port, SipTransportInfo::Type transport) const;

		bool IsItInList(const resip::Tuple &address) const;

		bool Empty() const;

		int  Size() const;

		QString Subnet(int index) const;

		int Port(int index) const;

	private:

		void AddIpNode(const resip::Data &ip,
			int mask,
			int port, 
			SipTransportInfo::IpVersion ipVersion,
			SipTransportInfo::Type transport);

		bool AddHostNode(const resip::Data &host, QString &error);

	private:

		class Node;
		typedef std::vector<Node> List;
		boost::scoped_ptr<List> m_list;
	};

	// ---------------------------------------------------------------

	class SipTrustedNodeList : boost::noncopyable
	{
	public:

		SipTrustedNodeList()
		{
			 m_isEnabled = true;
		}

		bool IsEnabled() const	{	return m_isEnabled;	}
		void Enabl(bool par)	{	m_isEnabled = par;	}

		bool IsTrustedNode(const QHostAddress &address, int port) const;
		bool IsTrustedNode(const resip::Tuple &address) const;

		SipNodeList& TrustedList()				{	return m_trustedList; }
		const SipNodeList& TrustedList() const	{	return m_trustedList; }
		SipNodeList& BanList()					{	return m_banList; }
		const SipNodeList& BanList() const		{	return m_banList; }

	private:
		bool m_isEnabled;
		SipNodeList m_trustedList;
		SipNodeList m_banList;
	};
}
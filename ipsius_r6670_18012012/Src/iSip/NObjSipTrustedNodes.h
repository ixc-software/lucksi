#pragma once

#include "stdafx.h"

#include "SipTrustedNode.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "Utils/IntToString.h"
#include "Utils/OutputFormatters.h"
#include "SipTransportInfo.h"

namespace iSip
{
	class NObjSipTrustedNodes : public Domain::NamedObject,
		public DRI::INonCreatable
	{
		Q_OBJECT;

	public:
		
		NObjSipTrustedNodes(Domain::NamedObject *pParent, 
			const Domain::ObjectName &name,
			SipNodeList &nodeList) :
			Domain::NamedObject(&pParent->getDomain(), name, pParent),
			m_nodeList(nodeList)
		{

		}

	// DRI
	public:

//		Q_PROPERTY(bool Enabled READ IsEnabled WRITE Enable);

		Q_PROPERTY(int Size READ Size);

		Q_INVOKABLE void AddNode(DRI::ICmdOutput *pOutput, 
			iSip::SipTransportInfo::Type transport,
			const QString &value, 
			int port = 0)
		{
			QString error;
			if (!m_nodeList.AddNode(value, port, transport, error))
			{
				pOutput->Add(error);
				return;
			}
		}

		// -------------------------------------------------------------------------------------

		Q_INVOKABLE void IsItTrustedNode(DRI::ICmdOutput *pOutput, const QString &par, int port = 0)
		{
			QHostAddress address;

			if (!address.setAddress(par))
			{
				pOutput->Add("Wrong value. Format: ...");
				return;
			}

			bool isItInList = true; //m_nodeList.IsItInList(address, port);

			if (isItInList)
				pOutput->Add(QString("Address '%1' is in list").arg(par));
			else
				pOutput->Add(QString("Address '%1' isn't in list").arg(par));

		}

		// -------------------------------------------------------------------------------------

		Q_INVOKABLE void DeleteNode(DRI::ICmdOutput *pOutput, int index)
		{
			if (index >= m_nodeList.Size())
			{
				pOutput->Add(QString("Wrong index value. List size %1").arg(m_nodeList.Size()));
				return;
			}

			m_nodeList.DeleteNode(index);
		}

		// -------------------------------------------------------------------------------------

		Q_INVOKABLE void ClearAll()
		{
			m_nodeList.ClearAll();
		}

		// -------------------------------------------------------------------------------------

		Q_INVOKABLE void Print(DRI::ICmdOutput *pOutput)
		{
			Utils::TableOutput table;
			
			table.AddRow("¹", "Subnet", "Port");

			if (m_nodeList.Empty()) 
				table.AddRow("-", "The list is empty");
			else
			{
				for (int i = 0; i < m_nodeList.Size(); ++i)
				{
					int port = m_nodeList.Port(i);

					table.AddRow(QString().setNum(i), 
						m_nodeList.Subnet(i), 
						(port == 0) ? "-": QString().setNum(port));
				}
			}

			pOutput->Add(table.Get());
		}
	
	private:

		int Size() const {	return m_nodeList.Size(); }
//		bool IsEnable() const	{	return m_nodeList.IsEnabled(); }
//		void Enable (bool par)	{	m_nodeList.Enabl(par); } 

	private:

		SipNodeList &m_nodeList;

	};
}


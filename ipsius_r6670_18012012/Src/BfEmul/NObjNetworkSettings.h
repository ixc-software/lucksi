#ifndef _NOBJ_NETWORK_SETTING_H_
#define _NOBJ_NETWORK_SETTING_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "Utils/HostInf.h"
#include "E1App/NetworkSettings.h"

namespace BfEmul
{

	class NObjNetworkSettings : public Domain::NamedObject,
		public DRI::INonCreatable
	{		
		Q_OBJECT;
	public:
		NObjNetworkSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name) :
			Domain::NamedObject(pParent, name, pParent)
		{
			m_useDHCP = false;
			m_mac = QNetworkInterface::allInterfaces().begin()->hardwareAddress();
		}

		Q_PROPERTY(bool DHCP READ DHCP WRITE m_useDHCP);
		bool DHCP() const { return m_useDHCP; }
		
		Q_PROPERTY(QString Ip READ m_ip WRITE Ip);
		void Ip(const QString &par)
		{
			Utils::HostInf::FromString(par.toStdString());
			m_ip = par;
		}
		Q_PROPERTY(QString Gateway READ m_gateway WRITE Gateway);
		void Gateway(const QString &par)
		{
			Utils::HostInf::FromString(par.toStdString());
			m_gateway = par;
		}

		Q_PROPERTY(QString Mask READ m_mask WRITE Mask);
		void Mask(const QString &par)
		{
			Utils::HostInf::FromString(par.toStdString());
			m_mask = par;
		}
		Q_PROPERTY(QString Mac READ m_mac WRITE m_mac);

		E1App::BoardAddresSettings GetNetworkSettings() const
		{
			E1App::BoardAddresSettings settings;

			if (m_useDHCP)
			{
				settings.UseDHCP(m_mask.toStdString());
			}
			else
			{
				settings.Set(m_ip.toStdString(), m_gateway.toStdString(), 
					m_mask.toStdString(), m_mac.toStdString());
			}
			return settings;
		}

	private:
		bool m_useDHCP;
		QString m_ip;
		QString m_gateway;
		QString m_mask;
		QString m_mac;
	};
};

#endif


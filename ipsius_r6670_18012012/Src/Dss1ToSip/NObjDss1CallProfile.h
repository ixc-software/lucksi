#pragma once

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"

namespace Dss1ToSip
{
	// profile for calls from user to dss1

	class NObjDss1InCallProfile : public Domain::NamedObject,
		public DRI::INonCreatable
	{
		
		Q_OBJECT;

	public:

		NObjDss1InCallProfile(Domain::NamedObject *pParent, const Domain::ObjectName &name) :
			NamedObject(&pParent->getDomain(), name, pParent)
		{
			m_callingAddrType	 = From;
			m_callingAddrNoDigit = ReleaseCall;
		}

		enum CallingAddrType
		{
			From,
			Contact
		};

		Q_ENUMS(CallingAddrType);

		Q_PROPERTY(CallingAddrType CallingAddrType READ m_callingAddrType WRITE m_callingAddrType);

		CallingAddrType GetCallingAddrType() const
		{
			return m_callingAddrType;
		}

		enum CallingAddrNoDigit
		{
			UsePilotNumber,
			ReleaseCall
		};
		Q_ENUMS(CallingAddrNoDigit);
		
		Q_PROPERTY(CallingAddrNoDigit CallingAddrNoDigit READ m_callingAddrNoDigit WRITE m_callingAddrNoDigit);		
		
		CallingAddrNoDigit GetCallingAddrNoDigit() const
		{
			return m_callingAddrNoDigit;
		}
		
		Q_PROPERTY(QString PilotNumber READ PilotNumber WRITE PilotNumber);
		
		std::string GetPilotNumber() const {	return m_pilotNumber; }

		void CheckCorrectProperty()
		{
			if(m_pilotNumber.empty() && m_callingAddrNoDigit == UsePilotNumber) 
			{
				ThrowRuntimeException("Pilot number is empty and m_callingAddrNoDigit == UsePilotNumber.");
			}
		}

	private:
		
		QString PilotNumber() const
		{
			return m_pilotNumber.c_str();
		}

		void PilotNumber(const QString &val)
		{
			m_pilotNumber = val.toStdString();
		}
	
	private:

		CallingAddrType m_callingAddrType;


		CallingAddrNoDigit m_callingAddrNoDigit;

		std::string        m_pilotNumber;

	};

}


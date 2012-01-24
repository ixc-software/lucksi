#pragma once

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "ObjLink/ObjectLinksServer.h"
#include "ISDN/IsdnLayersInterfaces.h"
#include "IHardwareL1.h"

namespace Dss1ToSip
{
	class NObjDss1;

	class NObjL1DriverForGate : public Domain::NamedObject,
		public DRI::INonCreatable,
		public ISDN::IL2ToDriver,
		public ObjLink::IObjectLinksHost,   // server 
		public ObjLink::IObjectLinkOwner,   // client
		public IHardwareToL1
    {
		Q_OBJECT;
    public:

        NObjL1DriverForGate(NObjDss1 &owner, 
			const Domain::ObjectName &name);
        ~NObjL1DriverForGate();
	
		enum TraceType 
		{ 
			State, // only state
			All,  // trace all packet
			Short,// trace all packet in short format
			Data  // trace only data packet
		};
        Q_ENUMS(TraceType);
		Q_PROPERTY(TraceType TraceType READ GetTraceType WRITE SetTraceType);
		Q_PROPERTY(bool HardwareActive READ HardwareActive);
		Q_PROPERTY(bool ActivatedByUser READ ActivatedByUser);

		IHardwareToL1 &HardwareToL1() {	return *this; } 
        ISDN::BinderToIDriver GetBinder();

	// property
	private:
		void SetTraceType(TraceType traceType);
		TraceType GetTraceType() const;
		bool HardwareActive() const
		{
			return m_isHardwareActive;
		}
		bool ActivatedByUser() const
		{
			return m_isActivatedByUser;
		}

	//Implementation IL2ToDriver:
	private:
		void DataRequest(QVector<Platform::byte> packet);
		void ActivateRequest();
		void DeactivateRequest();
		void PullRequest();
		void PullInd(QVector<Platform::byte> packet);
		void SetUpIntf(ISDN::BinderToIDrvToL2 pIntfUp);

	// IObjectLinksHost impl
	private:
		Utils::SafeRef<ObjLink::IObjectLinksServer> getObjectLinksServer();

	// IObjectLinkOwner impl
	private:
		void OnObjectLinkConnect(ObjLink::ILinkKeyID &linkID);
		void OnObjectLinkDisconnect(ObjLink::ILinkKeyID &linkID);
		void OnObjectLinkError(boost::shared_ptr<ObjLink::ObjectLinkError>);

	// IHardwareToL1
	private:
		void LinkHardware(Utils::SafeRef<IL1ToHardware> hardware);
		void UnlinkHardware(const IL1ToHardware *hardware);
		void Activated();
		void Deactivated();
		void DataInd(const std::vector<Platform::byte> &pack);

	private:
		void LogData(const void *data, int len, bool isSentPack);

    private:
		NObjDss1 &m_owner;
		NObjL1DriverForGate::TraceType m_traceType;
		iLogW::LogRecordTag m_sendTag;
		iLogW::LogRecordTag m_recTag;

		bool m_isHardwareActive;    // hardware уровень активен                                                    
		bool m_isActivatedByUser;   // активирован пользователем
		ObjLink::ObjectLinksServer  m_server;//server
		ObjLink::ObjLinkStoreBinder m_myLinkBinderStorage;
		ObjLink::ObjectLink<ISDN::IDriverToL2> m_user;
		Utils::SafeRef<IL1ToHardware> m_hardware; 
    };

}



#include "stdafx.h"
#include "iLog/LogBinaryUtils.h"
#include "NObjDss1.h"
#include "NObjL1DriverForGate.h"

namespace 
{
    const int CBinaryMaxBytes = 8;
    const int CBinaryMinBytes = 5;
};

namespace Dss1ToSip
{
	NObjL1DriverForGate::NObjL1DriverForGate(NObjDss1 &owner, 
		const Domain::ObjectName &name) :   
		NamedObject(&owner.getDomain(), name, &owner),
		m_owner(owner),
		m_traceType(State),
        m_sendTag(Log().RegisterRecordKind(L"Send", true)),
        m_recTag(Log().RegisterRecordKind(L"Rec", true)),
        m_isHardwareActive(false),
        m_isActivatedByUser(false),
        m_server( getDomain(), this ),
        m_user(*this)
    {}

	// -------------------------------------------------------------------------------

    NObjL1DriverForGate::~NObjL1DriverForGate()
    {
        ESS_ASSERT(m_hardware.IsEmpty());
    }

	// -------------------------------------------------------------------------------

	void NObjL1DriverForGate::SetTraceType(NObjL1DriverForGate::TraceType traceType)	
	{	
		m_traceType = traceType; 
	}

	// -------------------------------------------------------------------------------

	NObjL1DriverForGate::TraceType NObjL1DriverForGate::GetTraceType() const	
	{	
		return m_traceType; 
	} 

	// -------------------------------------------------------------------------------

    ISDN::BinderToIDriver NObjL1DriverForGate::GetBinder()
    {
        return m_myLinkBinderStorage.getBinder<ISDN::IL2ToDriver>(this);
    }

	// -------------------------------------------------------------------------------
	//Implementation IL2ToDriver:

    void NObjL1DriverForGate::DataRequest(QVector<Platform::byte> packet)
    {
        if(!m_isActivatedByUser || !m_isHardwareActive) return;
        
		LogData(packet.data(), packet.size(), true);
        
		m_hardware->DataReq(this, packet);
    }        

	// -------------------------------------------------------------------------------

    void NObjL1DriverForGate::ActivateRequest()                 		
    {                    
        ESS_ASSERT( m_user.Connected() );
        
        //ESS_ASSERT(!m_isActivatedByUser);

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Activate request" << iLogW::EndRecord;

		m_isActivatedByUser = true;

        if (m_isHardwareActive) m_user->ActivateConf();
    }

	// -------------------------------------------------------------------------------

    void NObjL1DriverForGate::DeactivateRequest() 								
    {
        ESS_ASSERT(m_isActivatedByUser);

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Driver deactivated" << iLogW::EndRecord;
       
        m_isActivatedByUser = false;
        m_user->DeactivateConf();
    }

	// -------------------------------------------------------------------------------

    void NObjL1DriverForGate::PullRequest()  									
    {
        /*Если канал свободен иначе ставит флаг БылЗапрос (в исходном коде)*/
        m_user->PullConf();
    }

	// -------------------------------------------------------------------------------

    void NObjL1DriverForGate::PullInd(QVector<Platform::byte> packet) 				
    {
        /*Если канал свободен, иначе добавляет в свою очередь*/
        DataRequest( packet); // там клонируется
    }

	// -------------------------------------------------------------------------------

	void NObjL1DriverForGate::SetUpIntf(ISDN::BinderToIDrvToL2 pIntfUp)
	{   
		if ( m_user.Connected() ) return;

		ESS_ASSERT ( pIntfUp->Connect(m_user) );            
	}

	// -------------------------------------------------------------------------------
	// IObjectLinksHost impl

    Utils::SafeRef<ObjLink::IObjectLinksServer> NObjL1DriverForGate::getObjectLinksServer()
    {
        return m_server.getInterface();
    }

	// -------------------------------------------------------------------------------
	// IObjectLinkOwner impl

    void NObjL1DriverForGate::OnObjectLinkConnect(ObjLink::ILinkKeyID &linkID)
	{}

	// -------------------------------------------------------------------------------

    void NObjL1DriverForGate::OnObjectLinkDisconnect(ObjLink::ILinkKeyID &linkID) 
    {
        ESS_ASSERT(m_user.Equal(linkID) && "Unexpected linkID");

		if (IsLogActive(InfoTag)) Log(InfoTag) << "ObjectLink to L2 was disconnected." << iLogW::EndRecord;

		m_isActivatedByUser = false;
    }
		
	// -------------------------------------------------------------------------------
    
	void NObjL1DriverForGate::OnObjectLinkError(boost::shared_ptr<ObjLink::ObjectLinkError>)
	{}        
		
	// -------------------------------------------------------------------------------
	// IHardwareToL1

	void NObjL1DriverForGate::LinkHardware(Utils::SafeRef<IL1ToHardware> hardware)
	{
		ESS_ASSERT(m_hardware.IsEmpty());

		m_hardware = hardware;
	}

	// -------------------------------------------------------------------------------

	void NObjL1DriverForGate::UnlinkHardware(const IL1ToHardware *hardware)
	{
		ESS_ASSERT(m_hardware.IsEqualIntf(hardware));

		m_hardware.Clear();

		Deactivated();
	}

	// -------------------------------------------------------------------------------

	void NObjL1DriverForGate::Activated()
	{
		if(m_isHardwareActive) return; 

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Activate indication" << iLogW::EndRecord; 

		m_isHardwareActive = true;
		ESS_ASSERT( m_user.Connected() );
		
		m_owner.L1Activated();
		if(m_isActivatedByUser) m_user->ActivateInd();
		

	}

	// -------------------------------------------------------------------------------

	void NObjL1DriverForGate::Deactivated()
	{
		if(!m_isHardwareActive) return; 

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Deactivate indication" << iLogW::EndRecord; 

		m_owner.L1Deactivated();
		if(m_isActivatedByUser) m_user->DeactivateInd();

		m_isHardwareActive = false;
	}

	// -------------------------------------------------------------------------------

    void NObjL1DriverForGate::DataInd(const std::vector<Platform::byte> &pack)
    {
        if(!m_isHardwareActive)     return;

        if(!m_isActivatedByUser)    return;

        ESS_ASSERT(m_user.Connected());
        
        QVector<Platform::byte> qPack;        
        Utils::Converter<>::DoConvert(pack, qPack);
        LogData(qPack.data(), qPack.size(), false);
        m_user->DataInd(qPack);
    }

	// -------------------------------------------------------------------------------

    void NObjL1DriverForGate::LogData(const void *data, int len, bool isSentPack)
    {
		if(!IsLogActive(InfoTag)) return;
        
		if(m_traceType == State) return;

		iLogW::LogRecordTag &tag = (isSentPack) ? m_sendTag : m_recTag;

        if(m_traceType == All) 
        {
            Log() << tag << iLogW::LogBinaryAll(data, len);
			return;
        }
        if(m_traceType == Data)
        {
            std::string out(iLogW::LogBinaryLarge(data, len, CBinaryMinBytes));
            if(!out.empty()) Log() << tag << out;
			return;
        }
        if(m_traceType == Short)
        {
            Log() << tag   
                << iLogW::LogBinaryShort(data, len, CBinaryMaxBytes);
        }
    }
};


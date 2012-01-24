#include "stdafx.h"

#include "Domain/DomainClass.h"

#include "isdnpack.h"
#include "isdnl3.h"

#include "L3Packet.h"
#include "L3PacketTypes.h"
#include "DssWarning.h"
#include "IeFactory.h"
#include "IeTypes.h"

namespace 
{
    const bool CAllwaysClearCalllList = true;
} // namespace 

namespace ISDN
{

    using Domain::IDomain;    

    IeConstants::Location IsdnL3::GetLocation()
    {
        return m_profile.GetOptions().m_Location;
    }


    //---------------------------------------------------------------------------------------------------

    IsdnL3::IsdnL3(IDomain& iDomain, IsdnInfra& infr, const L3Profile &profile) : 
        iCore::MsgObject( iDomain.getDomain().getMsgThread()),
        ILoggable(infr, profile.Name(), profile.getTraceOption().m_traceOn),
        m_profile(profile.ValidateFields()),
        m_logTags(*this),
        m_stat(m_profile.getStatActive()),
        m_domain(iDomain.getDomain()),
        m_pL2(*this),
        m_pMng(*this),
        m_stateByLinks(st_waitLinkConnection),
        m_thread( iDomain.getDomain().getMsgThread() ),
        m_iDomain(iDomain),
        m_infra(infr),                
        m_dssState(st_deactivated),
        m_ieFactory(infr),
        m_packFactory(m_infra, m_ieFactory),         
        m_callRefGen(m_profile.GetMaxCallrefValue(), m_calls),                
        m_distributor(m_profile.getIntfGroupe(), *this),
        T309(*this, StopAllTimers, this, &L3::ExpireT309, m_profile.GetTimers().T309 ),
        T316(*this, StopAllTimers, this, &L3::ExpireT316, m_profile.GetTimers().T316 ),
        m_timerReActivateReqL3(*this, StopAllTimers, this, &L3::ExpireTReActivateReq, m_profile.GetTimers().tReActivateReqL3 ),
        m_timerWaitFreeWinInd(*this, StopAllTimers, this, &L3::ExpireTWaitFreeWinInd, m_profile.GetTimers().tWaitFreeWinInd ),
        m_timerWaitFErr(*this, StopAllTimers, this, &L3::ExpireWaitFErr, m_profile.GetTimers().tProcessErrorF),
        m_FCounter(0),
        m_server(iDomain, this ),        
        m_recreated(false),
        m_calls(this, iDomain.getDomain().getMsgThread())         
    {                
    }

    //---------------------------------------------------------------------------------------------------

    // конструктор используемый при перезагрузке
    IsdnL3::IsdnL3(IDomain& iDomain, IsdnInfra& infr, const L3Profile &profile,
                   BinderToICallBackDss pMng, IObjectLinksHost* pL2Host, IL3ToL2* pL2Intf)
        : iCore::MsgObject( iDomain.getDomain().getMsgThread() ),         
        ILoggable(infr, profile.Name(), profile.getTraceOption().m_traceOn),
        m_profile(profile.ValidateFields()),
        m_logTags(*this),
        m_stat(m_profile.getStatActive()),
        m_domain(iDomain.getDomain()),
        m_pL2(*this),
        m_pMng(*this),
        m_stateByLinks(st_waitLinkConnection),
        m_thread( iDomain.getDomain().getMsgThread() ),
        m_iDomain(iDomain),
        m_infra(infr),                
        m_dssState(st_deactivated),
        m_ieFactory(infr),
        m_packFactory(m_infra, m_ieFactory),      
        m_callRefGen(m_profile.GetMaxCallrefValue(), m_calls),                
        m_distributor(m_profile.getIntfGroupe(), *this),
        T309(*this, StopAllTimers, this, &L3::ExpireT309, m_profile.GetTimers().T309 ),
        T316(*this, StopAllTimers, this, &L3::ExpireT316, m_profile.GetTimers().T316 ),
        m_timerReActivateReqL3(*this, StopAllTimers, this, &L3::ExpireTReActivateReq, m_profile.GetTimers().tReActivateReqL3 ),
        m_timerWaitFreeWinInd(*this, StopAllTimers, this, &L3::ExpireTWaitFreeWinInd, m_profile.GetTimers().tWaitFreeWinInd ),
        m_timerWaitFErr(*this, StopAllTimers, this, &L3::ExpireWaitFErr, m_profile.GetTimers().tProcessErrorF),
        m_FCounter(0),
        m_server(iDomain, this ),        
        m_recreated(true),
        m_calls(this, iDomain.getDomain().getMsgThread())       
    {        
        SetMngLink(pMng);
        SetL2Link(pL2Host, pL2Intf);
    }

    //---------------------------------------------------------------------------------------------------

    IsdnL3::~IsdnL3()
    {
        //std::cout << "*******Destructor IsdnL3\n";
        if (m_dssState != st_deactivated && m_dssState != st_goesActivate)
            m_pMng->Deactivated();
    }

    //---------------------------------------------------------------------------------------------------

    IsdnL3::LogRecordKinds::LogRecordKinds(ILoggable& logSession)
        : ClientMsg( logSession.RegNewTag("Msg from client") ),
          FromL2Msg( logSession.RegNewTag("Msg from layer2") ),
          ToL2Msg(logSession.RegNewTag("Msg to layer2") ),
          ObjLinkMsg( logSession.RegNewTag("ObjLink Inform") ),
          WarningMsg( logSession.RegNewTag("Warning") ),
          Error( logSession.RegNewTag("Error!") ),
          TimerEvents( logSession.RegNewTag("Timer Events") ),
          ExpandedInfo( logSession.RegNewTag("Expanded information") ),
          OwnAction( logSession.RegNewTag("Own Action") ),
          GeneralInfo( logSession.RegNewTag("General information") ),
          ChangeDssState( logSession.RegNewTag("Change Dss1 state") )
    {}

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ExpireT309(const EvT309& ev)
    {        
        SimpleLog("Event Expire T309", m_logTags.TimerEvents);
        ESS_ASSERT(m_dssState == st_goesActivate);

        StopAllTimers();
        m_calls.Clear( DssCause::Create( this, IeConstants::DestOutOfOrder ) );
        shared_ptr<const DssWarning> warning(L2NotActve::Create(m_infra, "reestablish failed (T309 expire)"));
        m_pMng->DssErrorMsg( warning );
        ChangeDssState(st_deactivated);
        //m_pMng->Deactivated();
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ExpireTReActivateReq(const EvTReActivateReq&)
    {
        ESS_ASSERT(m_dssState == st_goesActivate);        

        ESS_ASSERT( m_pL2.Connected() );
        if (m_timerReActivateReqL3.getRestRepeatCount() == 0 )
        {
            StopAllTimers();
            ChangeDssState(st_deactivated);

            shared_ptr<const DssWarning> warning(
                L2NotActve::Create(m_infra, "Can`t activate Layer2. ReActivationTimer do lastExpire")
                );
            m_pMng->DssErrorMsg( warning );

            
            if (m_profile.GetOptions().m_AutoReActivation) DoActivation();

            // неозначает переход из активного в неактивное состояние, но значительно упрощает логику пользователя
            //m_pMng->Deactivated(); 

            return;
        }
        SimpleLog("EstablishRequest", m_logTags.ToL2Msg);
        m_pL2->EstablishReq();
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ExpireTWaitFreeWinInd(const EvWaitFreeWindFailed&)
    {
        ESS_ASSERT(m_dssState == st_goesDeactivate);
        m_timerWaitFreeWinInd.Stop();
        SimpleLog("ReleaseReq", m_logTags.ToL2Msg);
        m_pL2->ReleaseReq();        
    }

    //---------------------------------------------------------------------------------------------------

    // links state assertion for methods ILayerDss implementation
    void IsdnL3::StateAssertForMngIntf()
    {        
        ESS_ASSERT(m_stateByLinks == st_noWait);
    }

    // links state assertion for methods IL2ToL3 implementation
    void IsdnL3::StateAssertForL2Intf()
    {     
        ESS_ASSERT(m_stateByLinks == st_noWait);
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::SetMngLink(shared_ptr<ObjLinkBinder<ICallbackDss> > pMng)//(ICallbackDss* pMng)
    {             
        ESS_ASSERT( !m_pMng.Connected() );
        ESS_ASSERT( pMng->Connect( m_pMng ) );        
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::SetL2Link (IObjectLinksHost* pHost, IL3ToL2* pIntf)
    {
        ESS_ASSERT(pIntf && pHost);
        ESS_ASSERT( m_stateByLinks != st_noWait );
        m_pL2.Connect(pHost, pIntf);
    }

   
    //---------------------------------------------------------------------------------------------------

    void IsdnL3::EstablishInd() 
    {   
        T309.Stop();
        m_timerReActivateReqL3.Stop();

        if (m_dssState == st_activated || m_dssState == st_restartReq || m_dssState == st_goesDeactivate)
        {
            SimpleLog("Layer2 ReEstablished", m_logTags.FromL2Msg);
            return;
        }
        
        ESS_ASSERT(m_dssState == st_deactivated || m_dssState == st_goesActivate);
        ESS_ASSERT(!m_timerWaitFreeWinInd.IsActive());

        StateAssertForL2Intf();
        SimpleLog("Establish Indication", m_logTags.FromL2Msg);
        
        //ESS_ASSERT m_pMng.Connected
        if (m_recreated && m_stateByLinks == st_noWait)
            m_recreated = false;
        
        //m_L2active = true;
        ChangeDssState(st_activated);        
        ESS_ASSERT(m_pMng.Connected());
        //m_pMng->Activated();
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::EstablishConf()
    {                
        // не критично но незаконно
        ESS_ASSERT(!m_timerWaitFreeWinInd.IsActive());        

        StateAssertForL2Intf();
        SimpleLog("Establish Confirmation", m_logTags.FromL2Msg);
        
        //if m_pL2->EstablishReq() and receive user cmd Deactivate
        if (m_dssState == st_goesDeactivate) return;        

        //ESS_ASSERT(m_pMng); ???
        if (m_recreated && m_stateByLinks == st_noWait)
            m_recreated = false;
        
        T309.Stop();
        m_timerReActivateReqL3.Stop();
        //TODO m_calls.Send Status or StatusEnqury
        //m_L2active = true;
        ChangeDssState(st_activated);     
        //m_pMng->Activated();
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ReleaseInd()
    {
        StateAssertForL2Intf();
        SimpleLog("Release Indication", m_logTags.FromL2Msg);
        
        if (m_dssState == st_deactivated) return;               

        if (m_dssState != st_restartReq && CAllwaysClearCalllList) 
            m_calls.Clear( DssCause::Create( this, IeConstants::DestOutOfOrder ) );

        if (m_dssState == st_goesActivate)
        {
            if (m_timerReActivateReqL3.IsActive()) return;

            StopAllTimers();            
            ChangeDssState(st_deactivated);
            Warning(shared_ptr<const DssWarning>(L2NotActve::Create(m_infra, "Can`t activate Layer2")));            
            return;
        }

        ESS_ASSERT(m_dssState == st_activated || m_dssState == st_goesDeactivate || m_dssState == st_restartReq);
        
        m_timerWaitFreeWinInd.Stop();                      
        
        if (!T309.IsIgnored())
        {
            ChangeDssState(st_goesActivate);
            m_pL2->EstablishReq();
            SimpleLog("EstablishRequest", m_logTags.ToL2Msg);        
            T309.Start();
        }
        else
        {
            ChangeDssState(st_deactivated);
            //m_pMng->Deactivated();
        }             

        //m_calls.Clear( DssCause::Create( &m_IL3, 27 ) ); // 27 - destination out of order
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::DataInd(QVector<byte> packet)
    {
        ESS_ASSERT(st_activated);
        StateAssertForL2Intf();
        
        m_stat.RxInc();

        if ( packet.size() < 3 ) // minimal data length
        {
            m_stat.BadPackInc();
            m_pMng->DssErrorMsg(shared_ptr<const DssWarning>( ShortPacket::Create(m_infra, packet) ));
			TraceDumpforIncomming(packet);
            return;
        }

        // TODO  if (pd == ....) //--// 
        shared_ptr<L3Packet> pL3Pack;

        // create/parse incoming data
        try
        {
            pL3Pack.reset( m_packFactory.CreateL3Pack(packet) );                
        }
        catch(const DssWarningExcept& msg)
        {
            TraceDumpforIncomming(packet);
			Warning(msg.getProperty());            
            m_stat.BadPackInc();
            return;
        }

        // trace packet
        if (getTraceOn() && m_profile.getTraceOption().m_traceDataExcangeL3)
        {
			LogWriter(this, m_logTags.FromL2Msg).Write()
				<< "Data Ind. " << pL3Pack->GetAsString(m_profile.getTraceOption().m_traceIeContent);
        }

        // если есть неизвестные Ie
        UnknownIeType* ieUnknown;
        if (pL3Pack->FindIe(ieUnknown))
        {
            shared_ptr<const PackWithUnknounIe> warning;
            std::stringstream ss;
            ieUnknown->WriteContent(ss);
            warning.reset( PackWithUnknounIe::Create(m_infra, packet, ss.str()) );
            m_pMng->DssErrorMsg(warning);

            if (getTraceOn())
                LogWriter(this, m_logTags.WarningMsg).Write()
                << "Packet with unsupported Ie: " 
                << pL3Pack->GetAsString(m_profile.getTraceOption().m_traceIeContent);
        }
            
        if (pL3Pack->UpRoutAsGlobal(*this/*, packet?*/)) return;		
        
        CallRef callref(pL3Pack->GetCallref());
        if (callref.IsGlobal()) 
        {            
            Warning(shared_ptr<DssWarning>(WrongPack::Create(m_infra, "Global CallRef in not global packet", packet)));
            m_stat.BadPackInc();
            //todo send status cause 81
            return;
        }

		L3Call *call = GetIL3Calls()->Find(callref);
		if(call == 0 && pL3Pack->Type() == L3MessgeType::SETUP)	call = GetIL3Calls()->CreateInCall(callref);

		if(call != 0) 
		{
			pL3Pack->UpRout(*call);
			return;
		}

		m_stat.BadPackInc();
		if (pL3Pack->Type() == L3MessgeType::SETUP)
		{
			Warning(shared_ptr<DssWarning>(CantCreateIncommingCall::Create( m_infra )));
		}
		else 
		{            
            PacketReleaseComplete pack(*this, callref); // pack.AddIe( IeCause(?) ) ?            
            Send(pack);
            
			Warning(shared_ptr<DssWarning>(WrongCallRefInNoSetupPack::Create(m_infra, std::string("Call ref: ") + callref.ToString())));
		}
    }

	//---------------------------------------------------------------------------------------------------

	void IsdnL3::UDataInd(QVector<byte>)
	{
		// ESS_UNIMPLEMENTED;
	}    

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::SetDownIntf(BinderToIL3ToL2 binderToL2)
    {
        SimpleLog("SetDownIntf", m_logTags.FromL2Msg);
        //ESS_ASSERT( !m_pL2.Connected() );
        if (!m_pL2.Connected())
            ESS_ASSERT( binderToL2->Connect( m_pL2 ) );
    }
    
    //---------------------------------------------------------------------------------------------------

    void IsdnL3::CreateOutCall(CreateOutCallParametrs params) 
    {
        StateAssertForMngIntf();
        SimpleLog("Create outgoing call.", m_logTags.ClientMsg);
        
        if ( st_activated != m_dssState )
        {                
            m_pMng->DssErrorMsg(
                shared_ptr<const DssWarning>(L2NotActve::Create(m_infra, "Try setup without active L2 cannel")) );
        }

        try
        {
            m_calls.CreateOutCall(params.m_bindToDssCall, params.m_callParams, params.m_unicCallName);
        }
        catch(const DssWarningExcept& msg)
        {
			Warning(msg.getProperty());
        }
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::Activate()
    {
        if (m_dssState != st_deactivated)
        {
            std::stringstream ss;
            ss << "Activate command was ignored (in state " << m_dssState << ")";
            m_pMng->DssErrorMsg(
                shared_ptr<const DssWarning>(DssNotDeactivated::Create(m_infra, ss.str())) );
            return;
        }
        StateAssertForMngIntf();
        SimpleLog("Activate", m_logTags.ClientMsg);                               
        
        DoActivation();
        //set timer?
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::DoActivation()
    {
        m_timerReActivateReqL3.Start();

        ESS_ASSERT( m_pL2.Connected() );
        SimpleLog("EstablishRequest", m_logTags.ToL2Msg);
        ChangeDssState(st_goesActivate);
        m_pL2->EstablishReq();
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::Deactivate()
    {
        if (m_dssState != st_activated)
        {
            m_pMng->DssErrorMsg(
                shared_ptr<const DssWarning>(DssNotDeactivated::Create(m_infra, "Deactivate command was ignored")) );
            return;
        }
        StateAssertForMngIntf();
        SimpleLog("Deactivate from user", m_logTags.ClientMsg);
        
        m_timerReActivateReqL3.Stop();

        T309.Stop();
        ChangeDssState(st_goesDeactivate);        

        // Деактивация L2 будет выполнена после того как все ReleaseComplete при AsyncClear будут отправленны.
        m_calls.AsyncClear( DssCause::Create(this, IeConstants::DestOutOfOrder) );        
    }

    //---------------------------------------------------------------------------------------------------
    

    void IsdnL3::AllCallsClearNotification()
    {
        ESS_ASSERT(m_dssState == st_goesDeactivate);
        ESS_ASSERT( m_pL2.Connected() );
        SimpleLog("All calls was closed", m_logTags.GeneralInfo);
        
        // FreeWinInd дает возможность убедится в том что все ReleaseComplete были отпраленны.
        m_timerWaitFreeWinInd.Start();        
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::FreeWinInd()
    {
        SimpleLog("FreeWinInd", m_logTags.FromL2Msg);
        if ( m_timerWaitFreeWinInd.IsActive() )
        {
            m_timerWaitFreeWinInd.Stop();
            SimpleLog("ReleaseReq", m_logTags.ToL2Msg);
            m_pL2->ReleaseReq();            
        }
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ReleaseConf()
    {
        if (m_recreated) return;
        ESS_ASSERT(m_dssState == st_goesDeactivate);

        SimpleLog("ReleaseConf", m_logTags.FromL2Msg);
        ChangeDssState(st_deactivated);
            
        m_timerWaitFreeWinInd.Stop();
        //m_pMng->Deactivated();
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ChangeDssState(DssState newState)
    {        
        if (getTraceOn())
            LogWriter(this, m_logTags.ChangeDssState).Write() << "from " << m_dssState << " to " << newState;
             
        if (m_dssState == newState) return;

        // activation
        if (newState == st_activated && m_dssState != st_restartReq/*is already active*/) 
        {
            m_stat.DssActivated();
            m_pMng->Activated();
        }

        // deactivation
        if (newState == st_deactivated && m_dssState != st_goesActivate) 
        {
            m_stat.DssDeactivated();       
            m_pMng->Deactivated();
        }

        if (m_dssState == st_restartReq) T316.Stop();        

        m_dssState = newState;               
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::OnObjectLinkConnect(ObjLink::ILinkKeyID &linkID) 
    {
        if (AllLinksConnected()) m_stateByLinks = st_noWait;
        //if ( m_pMng.Equal(linkID) ) - пользователю слать не надо тк он выполнял перезагрузку?
        if ( m_pL2.Equal(linkID) )
        {                
            SimpleLog("Link to Layer2 connected", m_logTags.ObjLinkMsg);

            m_pL2->SetUpIntf( m_myLinkBinderStorge.getBinder<IL2ToL3>(this) );
            SimpleLog("SetUpIntf", m_logTags.ToL2Msg);
        }
        else if(m_pMng.Equal(linkID))
        {
            SimpleLog("Link to client connected", m_logTags.ObjLinkMsg);                
        }
        else
        {
            ESS_ASSERT( 0 && "Unknown linkID");
        }
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::OnObjectLinkDisconnect(ObjLink::ILinkKeyID &linkID) 
    {        
        if ( m_pMng.Equal(linkID) )
        {            
            SimpleLog("Link to client disconnected", m_logTags.ObjLinkMsg);
            Deactivate();
            m_stateByLinks = st_waitLinkConnection;
            return;
        }
        if ( m_pL2.Equal(linkID) )
        {
            SimpleLog("Link to layer2 disconnected", m_logTags.ObjLinkMsg);
            ReleaseInd();         
            return;
        }
        ESS_ASSERT(0 && "Unknown linkID");
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::OnObjectLinkError(shared_ptr<ObjLink::ObjectLinkError> error) 
    {
        SimpleLog("OnError", m_logTags.ObjLinkMsg);        
    }

    //---------------------------------------------------------------------------------------------------

    IL3Calls* IsdnL3::GetIL3Calls() 
    { 
        return &m_calls;
    }

    //---------------------------------------------------------------------------------------------------

    ObjLink::ObjectLink<ICallbackDss>& IsdnL3::GetIDssMng()
    {
        return m_pMng;
    } 

    //---------------------------------------------------------------------------------------------------
    
    // отладочная фция
    void FillFixedPack(QVector<byte>& qData)
    {
        //qData.push_back(0x08); // single byte cref
        //qData.push_back(0x01);
        //qData.push_back(0x02);
        
        qData.push_back(0x08); // F2000 header 2byte cref
        qData.push_back(0x02); // cref len
        qData.push_back(0x00); // cref hi
        qData.push_back(0x02); // cref low 14
        
        qData.push_back(0x05); // Setup                                           

        qData.push_back(0x04); // BC - как у меня
        qData.push_back(0x03);
        qData.push_back(0x90);
        qData.push_back(0x90);
        qData.push_back(0xA3); 

        qData.push_back(0x18); // Ch ident
        qData.push_back(0x03);
        qData.push_back(0xA1);
        qData.push_back(0x83);
        qData.push_back(0x91); // мой 91 их 95

        qData.push_back(0x1e); // ProgressIndicator // у меня нет
        qData.push_back(0x02);
        qData.push_back(0x82); // IeConstants::m_Location::LN
        qData.push_back(0x83); // IeConstants::OrigAdrNonIsdn откуда известно

        //qData.push_back(0x6c); // CallingPN - вызывающий номер
        //qData.push_back(0x02); // у меня  - вызываемый
        //qData.push_back(0x41);
        //qData.push_back(0xc3);

        qData.push_back(0x70); // CalledPN == мой
        qData.push_back(0x03); // 
        qData.push_back(0x81);
        qData.push_back(0x31);
        qData.push_back(0x31);
    }
   

    void IsdnL3::Send(const L3Packet& l3Pack)
    {
        if (
            m_dssState != st_activated &&
            m_dssState != st_restartReq &&
            m_dssState != st_goesDeactivate
            ) return;
        

        if (getTraceOn() && m_profile.getTraceOption().m_traceDataExcangeL3)
		{
            LogWriter(this, m_logTags.ToL2Msg).Write() 
            << "Data req. " << l3Pack.GetAsString(m_profile.getTraceOption().m_traceIeContent); 
		}

		QVector<byte> qData;
        const bool CDebugSendFixed = false; // используется только для отладки

        if (CDebugSendFixed) 
			FillFixedPack(qData);
        else 
            Utils::Converter<>::DoConvert(l3Pack.GetRaw(), qData);
        

        m_stat.TxInc();

        m_pL2->DataReq(qData);
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::SendStackWarning(shared_ptr<const StackWarning> msg)
    {
        if (getTraceOn()) 
        {
            LogWriter(this, m_logTags.OwnAction).Write() << "Sending warning to client" << msg->ToString();
        }  
        m_pMng->DssErrorMsg(msg);
    }

    //---------------------------------------------------------------------------------------------------

    CallRefGenerator& IsdnL3::GetCallRefGen()
    {
        return m_callRefGen;
    }

    //---------------------------------------------------------------------------------------------------

    BChannelsDistributor& IsdnL3::GetBChanDistributor()
    {
        return m_distributor;
    }

    //---------------------------------------------------------------------------------------------------

    iCore::MsgThread& IsdnL3::GetThread()
    {
        return m_thread;
    }

    //---------------------------------------------------------------------------------------------------

    Domain::IDomain& IsdnL3::GetIDomain()
    {
        return m_iDomain;
    }

    //---------------------------------------------------------------------------------------------------

    IsdnInfra& IsdnL3::GetInfra()
    {        
        return m_infra;
    }    

    //---------------------------------------------------------------------------------------------------

    bool IsdnL3::TraceIeList()const 
    {
        return (m_profile.getTraceOption().m_traceIeList && getTraceOn());
    }

    //---------------------------------------------------------------------------------------------------

    bool IsdnL3::TraceIeContent()const
    {
        return (m_profile.getTraceOption().m_traceIeContent && getTraceOn());
    }
    
    //---------------------------------------------------------------------------------------------------

    bool IsdnL3::IsUserSide() const
    {
        return m_profile.IsUserSide();
    }

    //---------------------------------------------------------------------------------------------------

    const L3Profile::Options& IsdnL3::GetOptions()const
    {
        return m_profile.GetOptions();
    }

    //---------------------------------------------------------------------------------------------------

    const DssTimersProf& IsdnL3::GetTimersProf()const
    {
        return m_profile.GetTimers();
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ExpireWaitFErr( const EvFErrPeriod& )
    {
        // если за период таймера былb новые F то счетчик обгоняет число срабатываний
        if (m_timerWaitFErr.GetExpiredCount() < m_FCounter)  
        {
            if (m_timerWaitFErr.getRestRepeatCount() != 0) return;

            // try reestablish L2
            m_pL2->EstablishReq(); 

            shared_ptr<DssWarning> warning( MaxRepeatFError::Create(m_infra) ); 
            if (getTraceOn()) 
            {
                LogWriter(this, m_logTags.OwnAction).Write() << "Sending warning to client" << warning->ToString();
            }  
            m_pMng->DssErrorMsg(warning);            
        }        
        
        m_FCounter = 0;
        m_timerWaitFErr.Stop();        
    }

    void IsdnL3::ErrorInd( L2Error error )
    {   
        if (error.getErrorCode() == 'F')
        {
            m_FCounter++;
            m_timerWaitFErr.Start();
        }                
    }

    //---------------------------------------------------------------------------------------------------
    
	void IsdnL3::Warning(shared_ptr<const DssWarning> warning)
	{
		if (getTraceOn()) 
		{
			LogWriter(this, m_logTags.WarningMsg).Write() << warning->ToString();
		}
		m_pMng->DssErrorMsg(warning);
	}

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::TraceDumpforIncomming( QVector<byte> data )
    {
        if (getTraceOn() && m_profile.getTraceOption().m_traceDataExcangeL3)
        {
            LogWriter(this, m_logTags.FromL2Msg).Write() << "Data ind. " << Utils::DumpToHexString(data.data(), data.size());
        }
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ProcessGlobCrefPack( const PacketRestart* pack)
    {
        IeRestartInd* pInd;
        pack->FindIe(pInd);   

        PacketRestartAck ackPack(*this);
        
        // case #1 AllIntf
        if (pInd->getValue() == IeConstants::AllIntf)
        {
            m_calls.Clear( DssCause::Create( this, IeConstants::NormUnspec ) ); // cause ?
            
            ackPack.AddIe(*pInd);
            Send(ackPack);

            return;
        }

        IeChannelIdentification *pIeChannels;
        pack->FindIe(pIeChannels); 
        if (!pIeChannels )
        {                
            Warning( shared_ptr<DssWarning>(WrongPack::Create(m_infra, pack->GetAsString(true))) );
            return;
        }

        ackPack.AddIe(*pInd);
        ackPack.AddIe(*pIeChannels);

        // case #2 SingleIntf
        if (pInd->getValue() == IeConstants::SingleIntf)
        {                       
            if (!pIeChannels->GetIntfId().IsInited()) 
            {                
                Warning( shared_ptr<DssWarning>(WrongPack::Create(m_infra, pack->GetAsString(true))) );
                return;
            }           

            m_calls.CloseCalls(
                pIeChannels->GetIntfId().IsInited(),
                DssCause::Create(this, IeConstants::NormUnspec) // causenum ?
                );                                                
        }


        // case #3 Channels
        if (pInd->getValue() == IeConstants::Channels)
        {            
            m_calls.CloseCalls(
                pIeChannels->GetBchannels(),
                DssCause::Create(this, IeConstants::NormUnspec) // causenum ?
                );           
        }        

        Send(ackPack);
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ProcessGlobCrefPack( const PacketRestartAck* )
    {
        // todo send Status with cause 81, ignore
        if (m_dssState != st_restartReq)
        {
            Warning( shared_ptr<DssWarning>(WrongPack::Create(m_infra, "RestartAck without request.")) );
            return;
        }
        
        ChangeDssState(st_activated); // T316.Stop();
        m_pMng->RestartComplete(true);
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::ProcessGlobCrefPack( const PacketStatus* )
    {        
    }

    //---------------------------------------------------------------------------------------------------

    void IsdnL3::RestartReq()
    {
        StateAssertForMngIntf();
        SimpleLog("RestartReq.", m_logTags.ClientMsg);

        if ( st_activated != m_dssState )
        {                
            m_pMng->DssErrorMsg(shared_ptr<const DssWarning>                (
                L2NotActve::Create(m_infra, "Try RestartReq without active L2 channel. Command ignored.")) 
                );
            return;
        }

        RestartAllIntf();      
    }

    void IsdnL3::RestartAllIntf()
    {
        ESS_ASSERT(st_activated == m_dssState || st_restartReq == m_dssState);

        m_calls.Clear( DssCause::Create( this, IeConstants::NormUnspec ) );

        PacketRestart pack(*this);
        IeRestartInd ie(IeConstants::AllIntf);
        pack.AddIe(ie);
        ChangeDssState(st_restartReq);
        Send(pack);  
        T316.Start();
    }

    void IsdnL3::ExpireT316( const EvT316& )
    {       
        ESS_ASSERT(m_dssState == st_restartReq);

        if ( T316.getRestRepeatCount() == 0 )     
        {            
            ChangeDssState(st_activated);
            m_pMng->RestartComplete( ICallbackDss::CompleteInfo(false, "RecoveryTimerExpire") );        
        }
        else
            RestartAllIntf();
    }

    void IsdnL3::UpdateTraceOptions( const DssTraceOption& option )
    {
        m_profile.getTraceOption() = option;
        setTraceOn(option.m_traceOn);
    }

    void IsdnL3::ClearStat()
    {
        m_stat.Clear();        
        if (m_dssState == st_activated) m_stat.DssActivated();
    }
} // ISDN



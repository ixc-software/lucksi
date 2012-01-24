#include "stdafx.h"

#include "L3Call.h"
#include "iCore/MsgThread.h"//?

namespace 
{
	class CallMarker
	{
		Platform::dword m_lastMark;

		CallMarker()
			: m_lastMark(0)
		{}

	public:

		static CallMarker& Instance()
		{
			static CallMarker ret;
			return ret;
		}

		std::string CallName(const ISDN::CallRef &ref)
		{
			std::ostringstream out;
			out << "/L3Call_id_" << m_lastMark++ << "_cr_" << ref.ToString();
			return out.str();
		}

	};
}; // namespace

namespace ISDN
{

    L3Call::LogRecordKinds::LogRecordKinds(ILoggable& session) : 
		Info(session.RegNewTag("Info"))
    {}

    // Конструктор исходящего вызова        
    L3Call::L3Call(IIsdnL3Internal& IL3, BinderToUserCall link, shared_ptr<const DssCallParams> pParams, const std::string& callName) : 
        iCore::MsgObject( IL3.GetThread() ),        
        m_pOutParams( pParams ),
        m_isOutgoing(true),
        m_callRef( IL3.GetCallRefGen().GenCallRef() ),
        m_logSession(IL3.getParentSession(), CallMarker::Instance().CallName(m_callRef)),
        m_logTags(m_logSession),
        m_DownFsm(IL3, this, m_pInParams, m_pOutParams, m_linkToUserCall, &m_callRef),
        m_pCalls(IL3.GetIL3Calls()),         
        m_IL3(IL3),
        m_state(st_noWait),
        m_domain(IL3.GetIDomain().getDomain()),
        m_server(m_domain, this), //последовательность!
        m_linkToUserCall(*this)     
    {                                   
        if( !link->Connect(m_linkToUserCall) )
        {            
            shared_ptr<const DssWarning> warning(
                LinkToUserCallError::Create(m_IL3.GetInfra(), pParams) );
            ESS_THROW_T(DssWarningExcept, warning);
        }                   

        SimpleLog("Created", m_logTags.Info);
        LogCallInfo(callName);
    }

    L3Call* L3Call::CreateOutCall(IIsdnL3Internal& IL3, BinderToUserCall link, shared_ptr<const DssCallParams> pParams, const std::string& callName)
    {
        return new(IL3.GetInfra()) L3Call(IL3, link, pParams, callName);
    }

    //-------------------------------------------------------------------------------------------

    // конструктор входящего вызова
    L3Call::L3Call(const CallRef& cref, IIsdnL3Internal& IL3) :        
        iCore::MsgObject( IL3.GetThread() ),        
        m_isOutgoing(false),
        m_callRef(cref),
        m_logSession(IL3.getParentSession(), CallMarker::Instance().CallName(m_callRef)),
        m_logTags(m_logSession),
        m_DownFsm(IL3, this, m_pInParams, m_pOutParams, m_linkToUserCall, &m_callRef),
        m_pCalls(IL3.GetIL3Calls()),        
        m_IL3(IL3),
        m_state(st_noWait),
        m_domain(IL3.GetIDomain().getDomain()),
        m_server(m_domain, this), //последовательность!
        m_linkToUserCall(*this)     
    {        
        SimpleLog("Created", m_logTags.Info);        
    }

    L3Call* L3Call::CreateInCall(const CallRef& cref, IIsdnL3Internal& IL3)
    {
        return new(IL3.GetInfra()) L3Call(cref, IL3);
    }

    //-------------------------------------------------------------------------------------------

    L3Call::~L3Call()
    {        
        if (m_state != st_inDeleting && m_linkToUserCall.Connected())
        {
            m_state = st_inDeleting;            
            m_linkToUserCall->Disconnent(DssCause::Create( &m_IL3, IeConstants::NormUnspec ));
            m_linkToUserCall.Disconnect(); 
        }               

        if (m_logSession.getTraceOn())
            LogWriter(&m_logSession, m_logTags.Info).Write() << "Call deleted";        
    }
   
    //-------------------------------------------------------------------------------------------

    // Синхронное удаление( команда от списка вызовов или из OnObjLinkCon)
    void L3Call::DropCall(shared_ptr<const DssCause> pCause)    
    {        
        if (m_linkToUserCall.Connected())
        {
            m_linkToUserCall->Disconnent(pCause);
            m_linkToUserCall.Disconnect(); //LOCK        
        }

        // Send ReleaseComplete
        if ( !m_DownFsm.CurrStateIsNull() && m_state != st_inDeleting)
        {
            m_DownFsm.CCReleaseComplete(pCause); // послать Release или ReleaseComplete?        
        }        

        AsyncDeleteCall();
    }

    //-------------------------------------------------------------------------------------------

    void L3Call::CloseCall( shared_ptr<const DssCause> pCause ) // Override
    {
        ESS_ASSERT(m_DownFsm.CurrStateIsNull()); // ?
        if (m_linkToUserCall.Connected())
        {
            m_linkToUserCall->Disconnent(pCause);
            m_linkToUserCall.Disconnect(); //m_pL4.Lock();
        }
        
        AsyncDeleteCall();
    }

    //-------------------------------------------------------------------------------------------

    void L3Call::AsyncDeleteCall()
    {
        if (m_state == st_inDeleting) return; 
        
        m_state = st_inDeleting;
        m_pCalls->AsyncDeleteCall(this);
    }

    //-------------------------------------------------------------------------------------------

    void L3Call::ReleaseConfirm() // Override
    {        
        ESS_ASSERT(m_DownFsm.CurrStateIsNull()); // ?        
     
        AsyncDeleteCall();
    }       

    //-------------------------------------------------------------------------------------------

    // Вызывается после создания исходящего вызова
    void L3Call::SendSetupReq()
    {
        ESS_ASSERT( m_state == st_noWait );
        ESS_ASSERT(m_isOutgoing);        
        m_DownFsm.CCSetupReq();
    }

    //-------------------------------------------------------------------------------------------

    void L3Call::HoldReq()
    {
        //ESS_ASSERT( !m_stateWaitDeleteConfirm );        
    }

    //-------------------------------------------------------------------------------------------

    void L3Call::RetriveReq()
    {
        //ESS_ASSERT( !m_stateWaitDeleteConfirm );        
    }

    //-------------------------------------------------------------------------------------------

    void L3Call::RetriveConf() {/*TODO*/}

    //-------------------------------------------------------------------------------------------

    void L3Call::HoldConf() {/*TODO*/}

    //-------------------------------------------------------------------------------------------

    void L3Call::ConnectRsp()
    {
        // Cитуация когда пришел PacketReleaseComplete и fsm вызвала CloseCall
        if (st_inDeleting == m_state)
            return; // ignore

        ESS_ASSERT( m_state == st_noWait );
        m_DownFsm.CCSetupRsp();        
    }

    void L3Call::AsyncSetupCompl()
    {
        ESS_ASSERT( m_state == st_noWait ); // не уверен
        //Эта команда дается от имени пользователя, но её нет в интерфейсе
        //возможно это должно быть опцией.
        
        if(!m_IL3.IsUserSide()) PutMsg(this, &L3Call::SetupComplete);            
    }   

    void L3Call::SetupComplete()
    {
        //m_linkToUserCall->ConnectConfirm();
        m_DownFsm.CCSetupComplete();
    }

    BinderToDssCall L3Call::GetBinder()
    {
        return m_myLinkStorage.getBinder<IDssCall>(this);
    }               

    const ILoggable& L3Call::getLogSession()
    {
        return m_logSession;
    }

    void L3Call::LogCallInfo(const std::string& userCallName)
    {            
        if(!m_logSession.getTraceOn()) return;

		LogWriter(&m_logSession, m_logTags.Info).Write() << "Is "
			<< (m_isOutgoing ? "outgoing" : "incoming") << " call. " 
			<< (m_isOutgoing ? m_pOutParams->ToString() : m_pInParams->ToString())
            << (m_isOutgoing ? "Associated user call" : userCallName)
			<< " Call ref: " << m_callRef.ToString();
    }

    void L3Call::SimpleLog(const char* msg, iLogW::LogRecordTag kind)
    {
        if (m_logSession.getTraceOn()) m_logSession.DoLog(msg, kind);
    }

    const L3Call::LogRecordKinds& L3Call::getLogTags() const 
    {
        return m_logTags;
    }

    shared_ptr<const DssCallParams> L3Call::GetOutCallParams() 
    {
        return m_pOutParams;
    }        

    void L3Call::BChanBusyInSendSetup()
    {
        if ( m_linkToUserCall.Connected() )
        {
            m_linkToUserCall->Disconnent( DssCause::Create(&m_IL3, IeConstants::NoCircChanAvailable) );
            AsyncDeleteCall();
        }
        else
            m_state = st_waitOnLinkConnectedForDisc34;
    }

    const CallRef& L3Call::GetCallRef() const
    {
        return m_callRef;
    }        

    const BChannelsWrapper& L3Call::GetBCannels() const
    {
        ESS_ASSERT(m_setBChans.get()); // или return empty?
        return *m_setBChans; 
    }

    void L3Call::InitBCannels(const SharedBChansWrapper &set)
    {
        m_setBChans = set; 
    }

    //-------------------------------------------------------------------------------------------

    void L3Call::SetLink(BinderToUserCall binder)
    {        
        // Cитуация когда пришел PacketReleaseComplete и fsm вызвала CloseCall
        if (st_inDeleting == m_state)
            return; // ignore

        //if(m_state != st_noWait) //break
        ESS_ASSERT( m_state == st_noWait );
        
        // TODO ... stop SetLink-timer, change state
        
        if ( !binder->Connect(m_linkToUserCall) ) //set Link
        {   // фактически это команда Reject request от пользователя
            //TODO Reject option on user side            
            m_DownFsm.CCReleaseComplete( DssCause::Create(&m_IL3, IeConstants::CallRejected) );
            
            shared_ptr<const StackWarning> warning(
                LinkToUserCallError::Create(m_IL3.GetInfra(), m_pInParams) );
            m_IL3.SendStackWarning(warning);            
            
            AsyncDeleteCall();
        }
        // TODO дождатся коннекта?
        
        //m_DownFsm.CCProceedingReq(); // теперь это происходит после команды пользователя AddressComplete        
    }

    // -------------------------------------------------------------------------------------

    void L3Call::OnObjectLinkConnect(ILinkKeyID &linkID) 
    {
        if (m_state == st_waitOnLinkConnectedForDisc34)
        {
            shared_ptr<const DssCause> cause = DssCause::Create(&m_IL3, IeConstants::NoCircChanAvailable);
            m_state = st_noWait;
            DropCall(cause);            
        }

        if (m_state == st_noWait && m_isOutgoing )
        {
            SendSetupReq();
        }
    }

    // -------------------------------------------------------------------------------------

    void L3Call::OnObjectLinkDisconnect(ILinkKeyID &linkID) 
    {
        //TODO если в фсм не идет процесс разрыва или фсм не st_null,
        //то UserCall скоропостижно скончался те здесь наверно CCDiscReq or CCRelComplete
        if ( !m_DownFsm.CurrStateIsNull() 
            && m_state != st_waitRelByUser 
            &&  m_state != st_inDeleting )
        {
            // фактически это команда Reject request от пользователя
            //TODO Reject option on user side            
            m_DownFsm.CCReleaseComplete( DssCause::Create(&m_IL3, IeConstants::CallRejected) ); // N21 - call rejected            
           
            AsyncDeleteCall();
        }
    }

    // -------------------------------------------------------------------------------------

    void L3Call::Disconnent(  shared_ptr<const DssCause> pCause )
    {
        if (/*m_state == st_waitRelByCallList ||*/ m_state == st_waitRelByUser)
            return; // ignore
        m_linkToUserCall.Disconnect();  // блокировать отправку сообщений пользователю
        if ( m_DownFsm.CurrStateIsNull() )
        {
            if (st_inDeleting != m_state) AsyncDeleteCall();        
        }
        else
        {
            m_DownFsm.CCDiscReq(pCause);
            m_state = st_waitRelByUser;
            // TODO start timer tWaitRel
        }
    }

    // -------------------------------------------------------------------------------------
   
    void L3Call::Alerting()
    {
        // Cитуация когда пришел PacketReleaseComplete и fsm вызвала CloseCall
        if (st_inDeleting == m_state)
            return; // ignore

        ESS_ASSERT( m_state == st_noWait );
        m_DownFsm.CCAlertingReq();        
    }

    // -------------------------------------------------------------------------------------

    void L3Call::AddressIncomplete()
    {
        if (st_inDeleting == m_state)
            return; // ignore

        ESS_ASSERT( m_state == st_noWait );
        m_DownFsm.CCMoreInfoReq();
    }

    // -------------------------------------------------------------------------------------

    void L3Call::AddressComplete()
    {
        if (st_inDeleting == m_state)
            return; // ignore

        ESS_ASSERT( m_state == st_noWait );
        m_DownFsm.CCProceedingReq();
    }
    
    // -------------------------------------------------------------------------------------

    bool L3Call::CallRefEqual(const CallRef& callRef) const
    {
        return (callRef == m_callRef) && !m_DownFsm.CurrStateIsNull();
    }

    // -------------------------------------------------------------------------------------

    bool L3Call::HaveBChannels() const
    {
        return m_setBChans.get() != 0;
    }

    void L3Call::AsyncDisconnect( shared_ptr<const DssCause> pCause )
    {
        PutMsg(this, &L3Call::Disconnent, pCause);
    }

} // ISDN



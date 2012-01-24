
#include "stdafx.h"
#include "Domain/DomainClass.h"
#include "isdnl2.h"
#include "l2_impl.h"
#include "Utils/ErrorsSubsystem.h"


namespace ISDN
{
    using Domain::DomainClass;
    using iCore::MsgObject;
    using iCore::MsgThread;
    using iCore::MsgTimer;
    using ObjLink::ObjectLink;

    //-------------------------------------

    IsdnL2::IsdnL2( DomainClass& domain, BinderToIDriver pDrvBinder, IsdnInfra* pInfra, const shared_ptr<const L2Profile> prof)
        : MsgObject( domain.getMsgThread() ),
        ILoggable(*pInfra, prof->getLogSessionName(), prof->getTracedOn()),        
        m_logKinds(*this),
		m_pInfra(pInfra),
        m_stat(prof->getStatActive()),
		m_l2obj(prof, this),
        m_PV(*this, prof->getTracedRaw()),        
        m_domain( domain ),
		m_pIntfDown(*this),
		m_pIntfUp(*this),   //не известна на момент создания                   
        m_rnd(pInfra->GetSeed()),
        m_processingTimer(this, &IsdnL2::Process),
        m_server( domain, this )        
	{
        ESS_ASSERT( pDrvBinder->Connect( m_pIntfDown ) );
        m_linksState = st_waitLinkConnection;
        // TODO set state
		ESS_ASSERT(pInfra);
	}

    //-------------------------------------

    IsdnL2::IsdnL2( DomainClass& domain, BinderToIDriver pDrvBinder, BinderToIL2ToL3 l3Binder, IsdnInfra* pInfra, const shared_ptr<const L2Profile> prof)
        : MsgObject( domain.getMsgThread() ),
        ILoggable(*pInfra, prof->getLogSessionName(), prof->getTracedOn()),        
        m_logKinds(*this),        
        m_pInfra(pInfra),
        m_stat(prof->getStatActive()),
        m_l2obj(prof, this),
        m_PV(*this, prof->getTracedRaw()),        
        m_domain( domain ),
        m_pIntfDown(*this),
        m_pIntfUp(*this),   //не известна на момент создания                
        m_rnd(pInfra->GetSeed()),
        m_processingTimer(this, &IsdnL2::Process),
        m_server( domain, this )        
    {
        //TODO Asserts заменить наекую реакцию
        ESS_ASSERT( pDrvBinder->Connect( m_pIntfDown ) );
        ESS_ASSERT( l3Binder->Connect( m_pIntfUp ) );
        m_linksState = st_waitLinkConnection;
        // TODO set state
        ESS_ASSERT(pInfra);

        SimpleLog("Layer Was recreated", m_logKinds.general);        
    }

    //-------------------------------------

    IsdnL2::LogRecordKinds::LogRecordKinds(ILoggable& logSession)
        : objLink(logSession.RegNewTag("ObjectLinks proceeding")), // сообщения по процессу установки/разрыва ObjectLink`ов
        general(logSession.RegNewTag("General message")), // общие сообщения
        mdlErorrs(logSession.RegNewTag("MDL error message")), // сообщения MdlErrorManagment
        frameErrorInfo(logSession.RegNewTag("Frame error info")), // ошибки пакетов
        timerEvents(logSession.RegNewTag("Timer events")), // события таймеров
        warning(logSession.RegNewTag("Warning")),
        tei(logSession.RegNewTag("TeiManage"))
    {}

    //-------------------------------------
   
    ObjectLink<IL2ToL3>& IsdnL2::GetUpIntf()
	{
        if (st_upLinkDisconnected != m_linksState)
        {
            ESS_ASSERT(m_pIntfUp.Connected() && "Interface don`t set");                
        }
        if (!m_pIntfUp.Connected())
            SimpleLog(" Warning: use disconnected up interface.", m_logKinds.objLink);
        return m_pIntfUp;
	}

    //-------------------------------------

	ObjectLink<IL2ToDriver>& IsdnL2::GetDownIntf()
	{
        if (!m_pIntfDown.Connected())
		    ESS_ASSERT(m_pIntfDown.Connected() && "Interface don`t set");
        
        //if (m_linksState != st_noWait)
            ESS_ASSERT(m_linksState == st_noWait || st_downLinkDisconnected != m_linksState);
        return m_pIntfDown;
	}

    //-------------------------------------

    BinderToIDrvToL2 IsdnL2::GetBinderIDrvToL2()
    {
        return m_bindStorage.getBinder<IDriverToL2>(this);
    }

    //-------------------------------------

    // Запуск процессинга внутренних таймеров
    void IsdnL2::StartProcessing(int periodMsec)
    {
        m_processingTimer.Start(periodMsec, true);
    }

    // Останов процессинга внутренних таймеров
    void IsdnL2::StopProcessing()
    {
        m_processingTimer.Stop();
    }

    //-------------------------------------
    
    void IsdnL2::SetUpIntf(BinderToIL2ToL3 binderToL3)
    {
        if (!m_pIntfUp.Connected())
            ESS_ASSERT( binderToL3->Connect(m_pIntfUp) );
    }

    //-------------------------------------

    Utils::SafeRef<ObjLink::IObjectLinksServer> IsdnL2::getObjectLinksServer()
    {
        return m_server.getInterface();
    }

    //-------------------------------------

    DomainClass& IsdnL2::getDomain()
    {
        return m_domain;
    }

    //-------------------------------------

    bool IsdnL2::AllLinksIsConnected()
    {
        return m_pIntfDown.Connected() && m_pIntfUp.Connected();
    }

    //-------------------------------------

    void IsdnL2::OnObjectLinkConnect(ILinkKeyID &linkID) 
    {
        //ESS_ASSERT(m_linksState != st_noWait);

        if ( AllLinksIsConnected() ) 
            m_linksState = st_noWait;

        if ( m_pIntfDown.Equal(linkID) )
        {                
            m_pIntfDown->SetUpIntf( m_bindStorage.getBinder<IDriverToL2>(this) );
            SimpleLog("DownIntf connected. MyBinder send down.", m_logKinds.objLink);
            return;
        }

        if ( m_pIntfUp.Equal(linkID) )
        {         
            SimpleLog("UpIntf connected.", m_logKinds.objLink);

            m_pIntfUp->SetDownIntf( m_bindStorage.getBinder<IL3ToL2>(this) );
            SimpleLog("My binder send up.", m_logKinds.objLink);

            return;
        }
        ESS_ASSERT(0 && "Unknown linkID");
    }

    //-------------------------------------

    void IsdnL2::OnObjectLinkDisconnect(ILinkKeyID &linkID) 
    {            
        // интерпретируем потерю линка как команду деактивации с соответствующей стороны
        
        if ( m_pIntfDown.Equal(linkID) )
        {
            SimpleLog("DownIntf disconnected.", m_logKinds.objLink);

            m_linksState = st_downLinkDisconnected;
            DeactivateInd();
            return;
        }
        if ( m_pIntfUp.Equal(linkID) )
        {
            SimpleLog("UpIntf disconnected.", m_logKinds.objLink);                

            m_linksState = st_upLinkDisconnected;
            ReleaseReq();
            return;
        }
        ESS_ASSERT(0 && "Unknown linkID");
    }

    //-------------------------------------

    void IsdnL2::Process(MsgTimer*)
    {
        m_l2obj.pl2m->Process();
        m_l2obj.m_TeiManage.tei_m->Process();        
    }

    //-------------------------------------

    void IsdnL2::SimpleLog(const char* cstr, iLogW::LogRecordTag kind)
    {
        if ( getTraceOn() )
            DoLog(cstr, kind);
    }

    //-------------------------------------

    void IsdnL2::GetRandomBytes(void *buf, int nbytes)
    {
	    byte* p=static_cast<byte*>(buf);
	    for(int i=0; i<nbytes; ++i)
            p[i] = m_rnd.NextByte();
    }

    //-------------------------------------

	void IsdnL2::SetUpIntf(ObjLink::IObjectLinksHost* pHost, IL2ToL3* pIntf)
	{
        ESS_ASSERT(pHost && pIntf);
        m_pIntfUp.Connect(pHost, pIntf);
    }

    //-------------------------------------

    void IsdnL2::SetDownIntf(BinderToIDriver drvBinder)
    {
        ESS_ASSERT( !m_pIntfDown.Connected() );
        drvBinder->Connect(m_pIntfDown);
    }
	
    //--------------------------------------
	
	void IsdnL2::EstablishReq()
	{
        SimpleLog("EstablishReq from L3", m_logKinds.general);
        m_l2obj.EstabReq();
    }

    //-------------------------------------

	void IsdnL2::ReleaseReq()
	{
        SimpleLog("ReleaseReq from L3", m_logKinds.general);
        m_l2obj.RelReq();
    }

    //-------------------------------------

	void IsdnL2::DataReq(QVector<byte> l3data)
	{
        SimpleLog("DataReq from L3", m_logKinds.general);
        IPacket* pPacket = m_pInfra->CreatePacket<IPacket>();
        pPacket->Fill(l3data);
        //m_l2obj.DatReq( pPacket->Clone() );
        m_l2obj.DatReq( pPacket );
    }

    //-------------------------------------

	void IsdnL2::UDataReq(QVector<byte> l3data)
	{
        SimpleLog("UDataReq from L3", m_logKinds.general);
        UPacket* pPacket = m_pInfra->CreatePacket<UPacket>();
        pPacket->Fill(l3data);
        //m_l2obj.UDatReq( pPacket->Clone() );
        m_l2obj.UDatReq( pPacket );
    }
		
	//---------------------------------------------------------------------
	//MDL_xxx message:
	void IsdnL2::MAssignReq(int arg)
	{
        m_l2obj.MAsgnReq(arg);
    }

    //-------------------------------------
	
	void IsdnL2::MRemoveReq()
	{
        m_l2obj.MRmovReq();
    }

    //-------------------------------------
	
	void IsdnL2::MErrorResponse()
	{
        m_l2obj.MErrResp();
    }

    //---------------------------------------------------------------------
	//impl of IDriverToL2

	void IsdnL2::DataInd(QVector<byte> packet)
	{
        SimpleLog("Receive data from driver", m_logKinds.general);
        IsdnPacket* pPacket = m_pInfra->CreatePacket<IsdnPacket>();
        pPacket->Fill(packet);
        m_PV.Draw(pPacket, !m_l2obj.m_isUserSide /*.m_pRole->IsNetworkSide()*/);
        //m_l2obj.DatInd( pPacket->Clone() );
        m_l2obj.DatInd( pPacket );
    }

    //-------------------------------------
	
	void IsdnL2::ActivateInd()
	{
        SimpleLog("ActivateInd from driver", m_logKinds.general);
        m_l2obj.ActivInd();
    }

    //-------------------------------------
	
	void IsdnL2::DeactivateInd()
	{ 
        SimpleLog("DeactivateInd from driver", m_logKinds.general);
        DeactivateConf(); 
    }

    //-------------------------------------

	void IsdnL2::ActivateConf()
	{ 
        SimpleLog("ActivateConf from driver", m_logKinds.general);
        ActivateInd(); 
    }

    //-------------------------------------
	
	void IsdnL2::DeactivateConf()
	{
        SimpleLog("DeactivateConf from driver", m_logKinds.general);
        m_l2obj.DeactConf();
    }

    //-------------------------------------

	void IsdnL2::PullConf()
	{
        m_l2obj.PullCnf();
    }

    //-------------------------------------
	
	void IsdnL2::PauseInd()
	{
        m_l2obj.PausInd();
    }

    //-------------------------------------
	
	void IsdnL2::PauseConf()
	{
        m_l2obj.PausConf();
    }    

    //-------------------------------------------------------

	void IsdnL2::MErrorIndication(L2Error error)
	{
		if( getTraceOn() )
		{
            LogWriter(this, m_logKinds.mdlErorrs).Write() 
                << "-ManageErrorIndication receive error: " << error.getAsString();
		}
        m_pIntfUp->ErrorInd(error);
	}

    //-------------------------------------
    
	int IsdnL2::Layer2::BCStx_cntInc(int inc)
	{
        int ret = tx_cnt;
	    tx_cnt += inc; 
        return ret;
    }

}//namespace ISDN


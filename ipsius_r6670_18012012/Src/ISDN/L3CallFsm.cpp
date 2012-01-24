#include "stdafx.h"

#include "L3CallFsm.h"
#include "L3PacketTypes.h"
#include "IeTypes.h"
#include "SetBCannels.h"
#include "L3Call.h"
#include "L3StatData.h"
#include "BChannelsDistributor.h"


namespace ISDN
{

    using boost::shared_ptr;
    using ObjLink::ObjLinkBinder;

    // -----------------------------------------------------------------------

    L3CallFsm::L3CallFsm(IIsdnL3Internal& IL3, L3Call *pOwner, shared_ptr<const DssInCallParams>& pInParams,
        shared_ptr<const DssCallParams>& pOutParams, ObjLink::ObjectLink<IDssCallEvents>& pL4, const CallRef* cref) :        
        ILoggable(pOwner->getLogSession(), "/Fsm"),
        m_fsm(st_null, false, boost::bind(&T::OnNoEventHandle, this, _1), 
                              boost::bind(&T::OnStateChange, this, _1, _2) ),
        m_IL3(IL3),
        m_pOwner(pOwner),
        m_pIOwner(pOwner),
        m_pInParams(pInParams),
        m_pOutParams(pOutParams),
        m_linkToUserCall(pL4),        
        //m_logSession(m_pIOwner->getLogSession(), "/Fsm"),
        m_logTags(*this),
        m_isUserSide( m_IL3.IsUserSide() ),        
        T301(m_IL3, StopAllTimers, this, &Here::ProcessTimerEvent, m_IL3.GetTimersProf().T301 ),
        T302(m_IL3, StopAllTimers, this, &Here::ProcessTimerEvent, m_IL3.GetTimersProf().T302 ),
        T303(m_IL3, StopAllTimers, this, &Here::ProcessTimerEvent, m_IL3.GetTimersProf().T303 ),
        T304(m_IL3, StopAllTimers, this, &Here::ProcessTimerEvent, m_IL3.GetTimersProf().T304 ),
        T305(m_IL3, StopAllTimers, this, &Here::ProcessTimerEvent, m_IL3.GetTimersProf().T305 ),
        T308(m_IL3, StopAllTimers, this, &Here::ProcessTimerEvent, m_IL3.GetTimersProf().T308 ),
        T310(m_IL3, StopAllTimers, this, &Here::ProcessTimerEvent, m_IL3.GetTimersProf().T310 ),
        T313(m_IL3, StopAllTimers, this, &Here::ProcessTimerEvent, m_IL3.GetTimersProf().T313 )
    {
        m_fsm.EnableRecursiveMode();
        if (m_isUserSide) InitFsmTableUser(m_fsm);
                     else InitFsmTableNetwork(m_fsm);
    }

    void L3CallFsm::WriteTalkStat(States prevState, States newState)
    {
        if (Active_10 == prevState && Active_10 != newState)
            m_IL3.getStat().TalkDec();

        if (Active_10 == newState && Active_10 != prevState)
            m_IL3.getStat().TalkInc();
    }

    //---------------------------------------------------------------------------------------

    /*обработчики восходящих событий*/
    void L3CallFsm::RxAlerting(const PacketAlerting& pack)
    {
        if (m_isUserSide)
        {
            if (m_fsm.GetState() == CallInitiated_1)
            {
                //T301.Stop();
                ProcessSetupAck(pack);  // can close here  
                T303.Stop();

                m_fsm.SetState(CallDelivered_4);                
            }

            if (m_fsm.GetState() == OverlapSending_2)
                T304.Stop();
            if (m_fsm.GetState() == OutCallProceeding_3)
                T310.Stop();                        
        }

        else
        {
            if (m_fsm.GetState() == OverlapReceiv_25)
                T304.Stop();
            if (m_fsm.GetState() == InCallProc_9)
                T310.Stop();
            if (m_fsm.GetState() == CallPresent_6)
            {
                ProcessSetupAck(pack);
                T303.Stop();
            }
            T301.Start();
        }

        m_linkToUserCall->Alerting();
    }

    //---------------------------------------------------------------------------------------



    //---------------------------------------------------------------------------------------

    //void L3CallFsm::RxSetupAckOUT(const PacketSetupAck& pack)
    //{
    //    // .. Extract info..
    //    // .. Channel proc
    //    shared_ptr<SetBCannels> setCannels ( SetBCannels::Create(m_IL3.GetInfra(), m_pOutParams->GetAckBchans()) );
    //    m_pL4->DssCallCreated(m_pOwner, setCannels );
    //}

    //---------------------------------------------------------------------------------------

    void L3CallFsm::RxConnect(const PacketConnect& pack)
    {
        //TODO stop T310 (в стандарте есть если currState() == 3, а если == 4 -- нет. Есть в i4l)
        if (m_isUserSide)
        {
            if ( m_fsm.GetState() == OutCallProceeding_3)
                T310.Stop();
            if ( m_fsm.GetState() == OverlapSending_2)
                T304.Stop();

            m_linkToUserCall->Connected();
            if ( m_IL3.GetOptions().m_SendConnectAck )
                DoConnectAck( EvConnectAck() );
        }
        else
        {
            if ( m_fsm.GetState() == OverlapReceiv_25)
                T304.Stop();
            if ( m_fsm.GetState() == InCallProc_9)
                T310.Stop();
            if ( m_fsm.GetState() == CallReceive_7)
                T301.Stop();
            if ( m_fsm.GetState() == CallPresent_6)
            {
                T303.Stop();
                ProcessSetupAck(pack);
            }

            m_linkToUserCall->Connected();
            m_pIOwner->AsyncSetupCompl();//send connect ack
        }
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::RxConnectAck(const PacketConnectAck& pack)
    {
        if (m_isUserSide)
        {
            T313.Stop();
            m_linkToUserCall->ConnectConfirm();
        }
        else
        {
            //// этого нет в стандарте:
            //if ( m_IL3.GetOptions().m_SendConnectAck )
            m_linkToUserCall->ConnectConfirm();
        }
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::RxSetupIn(const PacketSetup& pack)
    {
        //TODO disc if no idle channel

        IeChannelIdentification *pChanIe;        

        if ( !pack.FindIe(pChanIe) )
        {
            // todo send Status cause 96?
            ESS_THROW_T(
                ErrPacketProcess, 
                shared_ptr<ErrActionCloseCall>(new ErrActionCloseCall(IeConstants::MndIeIsMissing))
                );    
        }        		
		          
	    shared_ptr<IeChannelIdentification> pChanIeForResponse = 
		    m_IL3.GetBChanDistributor().CreateResponse(pack, m_pOwner);
        if (pChanIeForResponse == 0)
        {
            //ESS_ASSERT(0 && "Inquired channels Busy TODO");
            EvRelComplete ev( DssCause::Create(&m_IL3, IeConstants::NoRequestedCircChanAvailable) );
            DoRelCompete( ev );
            m_pIOwner->CloseCall( ev.m_cause );
            return;
        }
        ESS_ASSERT(m_IeChanForFirstResponse == 0);
        m_IeChanForFirstResponse = pChanIeForResponse;                    

        m_pInParams.reset( DssInCallParams::Create( m_IL3.GetInfra(), pack, m_pIOwner->GetBCannels() ) );
        m_pIOwner->LogCallInfo();        

        ICallbackDss::IncommingCallParametrs params(m_pIOwner->GetBinder(), m_pInParams, m_pIOwner->getLogSession().getName());
        ESS_ASSERT(m_pIOwner->GetBCannels().getChans().Count());
        m_IL3.GetIDssMng()->IncomingCall( params ); //CC setup indication

        // после этого ожидается (таймер ожидания) ответ пользователя AddressComplete/AddresIncomplete
        // в зависимости от ответа будет послан setupAck(CCMoreInfoReq) или callProceeding        
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::RxProceeding(const PacketProceeding& pack)
    {        
        ProcessSetupAck(pack);         

        //TODO stop BCannelsTimer (free requested channels) ?
        if (m_isUserSide)
        {                                    
            if (m_fsm.GetState() == CallInitiated_1)
                T303.Stop();
            if (m_fsm.GetState() == OverlapSending_2)
                T304.Stop();
            T310.Start();                        
        }
        else // network side
        {                                                               
            if (m_fsm.GetState() == CallPresent_6)
                T303.Stop();
            if (m_fsm.GetState() == OverlapReceiv_25)
                T304.Stop();
            T310.Start();                     
        }        
    }

    // ------------------------------------------------------------------------------------

    void L3CallFsm::ProcessSetupAck( const L3Packet& pack )
    {        
        IeChannelIdentification* pIeChanId;
        if (!pack.FindIe(pIeChanId))
        {
            IeConstants::CauseNum causeNum = 
                m_fsm.GetState() == CallInitiated_1 ?
                IeConstants::MndIeIsMissing :
            IeConstants::InvalidIeContent;               

            // todo send status ?            
            ESS_THROW_T(
                ErrPacketProcess, 
                shared_ptr<ErrActionCloseCall>(new ErrActionCloseCall(causeNum))
                );   
        }

        if ( 
            pIeChanId->GetChanSelection() == IeConstants::NoChannel ||
            pIeChanId->GetChanSelection() == IeConstants::AnyChannel &&
            m_pOutParams->GetMode() == IeConstants::Circuit &&
            m_pOutParams->GetRate() == IeConstants::KbPs64
            )
            ESS_THROW_T(
            ErrPacketProcess, 
            shared_ptr<ErrActionCloseCall>(new ErrActionCloseCall(IeConstants::InvalidIeContent))
            ); 

        ProcessChanResponse(pack);
        
        IDssCallEvents::DssCallCreatedParametrs params(m_pIOwner->GetBinder(), m_pIOwner->GetBCannels());
        m_linkToUserCall->DssCallCreated(params);          
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::RxDisconnect(const PacketDisc& pack)
    {
        IeCause* pIeCause;

        //эта ошибка должна обработатся в констр пакета
        if( !pack.FindIe(pIeCause) )
        {
            // todo send status.
            ESS_THROW_T(
                ErrPacketProcess, 
                shared_ptr<ErrActionCloseCall>(new ErrActionCloseCall(IeConstants::MndIeIsMissing))
                );             
        }        
        m_cause = pIeCause->GetCause();

        StopAllTimers();
        if (m_isUserSide)
        {
            //TODO if state 0,1,6,11,12,15,17,19 (i4l) ignore?
            m_fsm.SetState(DiscIndication_12);

            // TODO команда m_pL4->Disconnent() дублируется! при приеме PackRelCompl это правильно?
            //if (!m_pL4.IsEmpty() && !m_pL4.IsLocked())//page217
            //    m_pL4->Disconnent(m_cause);
        }
        else
        {
            //TODO if state 0,11,12,19,22 ignore?
            m_fsm.SetState(DiscRequested_11);
        }
        CCReleaseReq();
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::RxRelease(const PacketRelease& release)
    {
        IeCause* pIeCause;
        
        if(m_linkToUserCall.Connected())
        {
            shared_ptr<const DssCause> cause;
            if ( release.FindIe(pIeCause) ) cause = pIeCause->GetCause();

            m_linkToUserCall->Disconnent( cause );
        }

        PacketReleaseComplete pack( m_IL3, m_pOwner->GetCallRef());
        Send(pack);

        if (m_fsm.GetState() == st_null) // не зависит от стороны
            return;

        //TODO Смело можно остановить все таймера (попробовать на финише)
        StopAllTimers();
        
        m_fsm.SetState(st_null);
        m_pIOwner->ReleaseConfirm();
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::RxInfo(const PacketInfo& pack)
    {
        // send info to client, state no change (OverlapSending)
        // ...
        T302.Stop();

        IDssCallEvents::Info info;

        IeCalledPartyNumber *pIeCalled;
        if (pack.FindIe(pIeCalled))
        {
            info.Num = pIeCalled->GetNum();            
        }
        else
        {
            return; // если нет вызываемого неочем отчитыватся
        }

        SendingComplete *pIeComplete;
        info.SendingComplete = (pack.FindIe(pIeComplete)) ? true : false;

        /*if (!info.SendingComplete) */T302.Start();
        m_linkToUserCall->MoreInfo(info);

        // todo signal
    }

    //-------------------------------------------------------------------------------------

    void L3CallFsm::RxRelComplete(const PacketReleaseComplete& pack)
    {
        if (m_fsm.GetState() == st_null) return;
        T308.Stop(); //непонятно какие таймера надо стопорить!
        
        m_fsm.SetState(st_null);

        IeCause* pIeCause;
        if( pack.FindIe(pIeCause) )
            m_cause = pIeCause->GetCause();

        m_pIOwner->CloseCall(m_cause);
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::RxProgress(const PacketProgress& pack)
    {
        // net side
        if (m_fsm.GetState() == InCallProc_9) T310.Stop();
        if (m_fsm.GetState() == OverlapReceiv_25) T304.Stop();

        // user side
        if (m_fsm.GetState() == OutCallProceeding_3) T310.Stop();
        if (m_fsm.GetState() == OverlapSending_2) T304.Stop();
        

        IeProgressInd* pInd;
        if (!pack.FindIe(pInd))
        {
            // todo send Status cause 96?
            ESS_THROW_T(
                ErrPacketProcess, 
                shared_ptr<ErrActionCloseCall>(new ErrActionCloseCall(IeConstants::MndIeIsMissing))
                );            
        }
        
        m_linkToUserCall->ProgressInd(pInd->getDescr());
    }

    // ------------------------------------------------------------------------------------

    void L3CallFsm::RxSetupAck( const PacketSetupAck& pack )
    {		
        ESS_ASSERT(m_fsm.GetState() == CallInitiated_1);
        ESS_ASSERT(m_isUserSide);

        ProcessSetupAck(pack);         

        T303.Stop();
        T304.Start();                                     
    }

    //---------------------------------------------------------------------------------------

    /* Обработчики исходящих событий*/
    void L3CallFsm::DoSetupReq(const EvSetupReq&)
    {
        /*TODO это для user, для net возможно не шлем каналы */

        PacketSetup pack(m_IL3, m_pOwner->GetCallRef());
                
        BChannelsDistributor& bDistr = m_IL3.GetBChanDistributor();

        SendingComplete ieSendComplete;
        pack.AddIe(ieSendComplete);

        // создаем запрос m_IeChanForInquiry если это первый setup
        if ( m_fsm.GetState() == st_null )
        {
            shared_ptr<IeChannelIdentification>
                pIeChanForInquiry( bDistr.CreateInquiry(m_pIOwner->GetOutCallParams(), m_pOwner) );

            if (pIeChanForInquiry == 0)
            {
                m_pIOwner->BChanBusyInSendSetup();
                return;
            }

            ESS_ASSERT(m_IeChanForInquiry.get() == 0);
            m_IeChanForInquiry = pIeChanForInquiry;// запоминаем
        }
        
        IeConstants::TransCap transCap = m_pIOwner->GetOutCallParams()->GetTransCap();
        IeConstants::TransMode transMode = m_pIOwner->GetOutCallParams()->GetMode();                        
        IeConstants::TransRate transRate = IeConstants::KbPs64;      
        IeConstants::UserInfo payload = m_pIOwner->GetBCannels().getPayload();

        BearerCapability bc(IeConstants::CCITT, transCap, transMode, transRate, payload);        

        pack.AddIe(bc);        

        pack.AddIe(*m_IeChanForInquiry);

        // ProgInd
        if (m_IL3.GetOptions().m_SendProgInd)
        {
            IeProgressInd progress(m_IL3.GetOptions().m_Location, m_pIOwner->GetOutCallParams()->GetProgressDescription());        
            pack.AddIe(progress);
        }

        IeCalledPartyNumber ieCalledNum( m_pIOwner->GetOutCallParams()->GetCalled() );
        pack.AddIe(ieCalledNum);

        const DssPhoneNumber& callingNum = m_pIOwner->GetOutCallParams()->GetCalling();               
        if (!callingNum.Digits().IsEmpty() && m_IL3.GetOptions().m_SendCallingIfExist)
        {
            IeCallingPartyNumber ieCallingNum( callingNum );
            pack.AddIe(ieCallingNum);    
        }                        

        Send(pack);

        T303.Start();
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoProceedingReq(const EvProceedingReq&)
    {        
        PacketProceeding pack(m_IL3, m_pOwner->GetCallRef());
        // каналы шлем только в первом ответе

        if (m_isUserSide)
        {
            if (m_fsm.GetState() != OverlapReceiv_25) // признак того что это первый ответ 
            {
                ESS_ASSERT(m_IeChanForFirstResponse.get() != 0);
                pack.AddIe(*m_IeChanForFirstResponse);
                m_IeChanForFirstResponse.reset();
            }
            else
                T302.Stop();
        }
        else
        {
            if (m_fsm.GetState() != OverlapSending_2) // признак того что это первый ответ 
            {
                ESS_ASSERT(m_IeChanForFirstResponse.get() != 0);
                pack.AddIe(*m_IeChanForFirstResponse);
                m_IeChanForFirstResponse.reset();
            }
            else
                T302.Stop();
        }               

        Send(pack);
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoAlertingReq(const EvAlertingReq&)
    {
        PacketAlerting pack(m_IL3, m_pOwner->GetCallRef());
        //TODO if state 6 user use chan id ie
        Send(pack);
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoSetupResponse(const EvSetupRsp&)///!
    {
        PacketConnect pack(m_IL3, m_pOwner->GetCallRef());
        //TODO 6 user B-Chan        
        //TODO скан обмена здесь имел ProgressInd

        if (m_IeChanForFirstResponse.get() != 0)
        {
            pack.AddIe(*m_IeChanForFirstResponse);
            m_IeChanForFirstResponse.reset();
        }

        Send(pack);
        T313.Start();
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoSetupComplete(const EvSetupComplete& ev)
    {
        DoConnectAck( EvConnectAck() );
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoInfoReq(const EvMoreInfoReq& ev)
    {
        ESS_ASSERT(m_IeChanForFirstResponse.get() != 0);
        PacketSetupAck setupAck( m_IL3, m_pOwner->GetCallRef());                
        setupAck.AddIe( *m_IeChanForFirstResponse );
        m_IeChanForFirstResponse.reset();
        Send(setupAck);

        T302.Start(); // таймер ожидания PacketInfo
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoConnectAck(const EvConnectAck&)
    {
        PacketConnectAck pack(m_IL3, m_pOwner->GetCallRef());
        Send(pack);
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoDiscReq( const EvDiscReq& ev ) // !!!manualy
    {
        PacketDisc pack(m_IL3, m_pOwner->GetCallRef());
        IeCause ieCause(ev.m_cause);
        pack.AddIe(ieCause);
        Send(pack);

        if(m_isUserSide)
        {
            //TODO if state != 1-10,25 ignore, but timer...
            T305.Start();
        }
        else
        {
            // TODO disconnect B channels
            StopAllTimers();
            T305.Start();
        }

        ///
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoReleaseReq(const EvReleaseReq& ev)
    {
        //ESS_ASSERT(!m_isUserSide);//временно
        PacketRelease pack(m_IL3, m_pOwner->GetCallRef());
        if (ev.m_cause.get())
        {
            IeCause ie(ev.m_cause);// ?? gcc ok?
            pack.AddIe( ie );
        }

        Send(pack);
        T308.Start();
    }
  
    //---------------------------------------------------------------------------------------

    void L3CallFsm::DoRelCompete(const EvRelComplete& ev)
    {
        PacketReleaseComplete pack(m_IL3, m_pOwner->GetCallRef());
        if (ev.m_cause.get())
        {
            IeCause ie(ev.m_cause);
            pack.AddIe( ie );
        }

        Send(pack);
        m_fsm.SetState(st_null);
        // delete this?
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ExpireT301(const EvT301& ev)
    {
        LogEvent("Expire T301");
        T301.Stop();
        SendTimeoutInd("Timeout of T301");        
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ExpireT302(const EvT302& ev)
    {
        T302.Stop();
        SendTimeoutInd("Timeout of T302");        
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ExpireT304(const EvT304& ev)
    {
        LogEvent("Expire T304");
        T304.Stop();
        if (m_isUserSide)
        {
            DoDiscReq( DssCause::Create(&m_IL3, IeConstants::RecoveryTimerExpire) );// TODO add diagnostic (timer num)
            SendWarning<SetupConfirmErr>();
        }
        else
        {
            SendTimeoutInd("Timeout of T304");
        }
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ExpireT303(const EvT303& ev)
    {
        LogEvent("Expire T303");
        if (m_isUserSide)
        {
            if ( ev.pTimer->GetExpiredCount() == 1 )
            {
                DoSetupReq( EvSetupReq() );
                //m_fsm.SetState(CallInitiated_1); - таким и было
            }
            else
            {
                T303.Stop();
                m_fsm.SetState(st_null);
                m_pIOwner->CloseCall( DssCause::Create(&m_IL3, IeConstants::RecoveryTimerExpire) ); // cause N?
            }
        }
        else
        {
            //TODO тоже, но сетуп без каналов           

            ESS_ASSERT(m_fsm.GetState() == CallPresent_6); // пока вижу что сюда попадаю только  в этом состоянии

            if ( ev.pTimer->GetExpiredCount() == 1 )
            {
                DoSetupReq( EvSetupReq() ); // возможно сетуп надо слать без каналов
            }
            else
            {
                T303.Stop();
                m_fsm.SetState(st_null);
                m_pIOwner->CloseCall( DssCause::Create(&m_IL3, IeConstants::RecoveryTimerExpire) ); // cause N?
            }            
        }
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ExpireT305(const EvT305&)
    {
        LogEvent("Expire T305");
        
        if (m_isUserSide)
        {
            DoReleaseReq( EvReleaseReq(m_cause) ); // T308.Start() here
            m_cause.reset();            
        }
        else
        {
            SendTimeoutInd("Timeout of T305");                     
            // T308.Start() here
            DoReleaseReq( 
                EvReleaseReq(DssCause::Create(&m_IL3, IeConstants::RecoveryTimerExpire))
                );            
        }

        T305.Stop();
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ExpireT308(const EvT308& ev) // 5.3.4.3
    {
        LogEvent("Expire T308");
        //user == network

        if (ev.pTimer->GetExpiredCount()  == 1)
        {
            DoReleaseReq( EvReleaseReq() );
        }
        else
        {
            T308.Stop();
            m_fsm.SetState(st_null);
            SendWarning<ReleaseConfirmErr>();
            m_pIOwner->CloseCall( DssCause::Create(&m_IL3, IeConstants::RecoveryTimerExpire) ); // cause N?
        }
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ExpireT310(const EvT310&)
    {
        LogEvent("Expire T310");
        //user == network

        T310.Stop();
        DoDiscReq( DssCause::Create(&m_IL3, IeConstants::RecoveryTimerExpire) );// TODO add diagnostic (timer num)
        if (m_isUserSide)
            SendWarning<SetupConfirmErr>();
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ExpireT313(const EvT313&)
    {
        LogEvent("Expire T313");
        ESS_ASSERT( m_isUserSide );

        T313.Stop();
        DoDiscReq( DssCause::Create(&m_IL3, IeConstants::RecoveryTimerExpire) );// TODO add diagnostic (timer num)
        SendWarning<ConnectConfirmErr>();
    }

    //---------------------------------------------------------------------------------------

    void L3CallFsm::ProcessChanResponse( const L3Packet& pack )
    {
        BChannelsDistributor& distr = m_IL3.GetBChanDistributor();
        if (  distr.ProcessResponse( pack, m_pOwner)  ) return;        

        IeConstants::CauseNum causeNum = IeConstants::NoCircChanAvailable;
        EvRelComplete ev( DssCause::Create(&m_IL3, causeNum) );
        DoRelCompete( ev );
        ESS_THROW_T(
            ErrPacketProcess, 
            shared_ptr<ErrActionCloseCall>(new ErrActionCloseCall(causeNum))
            );                             
    }

    // -------------------------------------------------

    void L3CallFsm::ProcessException( const ErrPacketProcess& e )
    {
        // any state !!!
        e.getProperty()->DoAction(*this);
    }     

    // -------------------------------------------------

    void L3CallFsm::InitFsmTableUser( Fsm &fsm )
    {
        #define ADD UTILS_FSM_ADD

        // ** Downstate **
        ADD(st_null,            EvSetupReq,       DoSetupReq,      CallInitiated_1);
        ADD(_AnyState_,         EvDiscReq,        DoDiscReq,       DiscRequested_11);
        ADD(CallPresent_6,      EvAlertingReq,    DoAlertingReq,   CallReceive_7);
        ADD(CallReceive_7,      EvSetupRsp,       DoSetupResponse, ConnectReq_8);
        ADD(CallPresent_6,      EvSetupRsp,       DoSetupResponse, ConnectReq_8);
        ADD(InCallProc_9,       EvSetupRsp,       DoSetupResponse, ConnectReq_8);
        ADD(CallPresent_6,      EvProceedingReq,  DoProceedingReq, InCallProc_9);
        ADD(InCallProc_9,       EvAlertingReq,    DoAlertingReq,   CallReceive_7);
        ADD(DiscIndication_12,  EvReleaseReq,     DoReleaseReq,    ReleaseReq_19);
        ADD(CallPresent_6,      EvMoreInfoReq,    DoInfoReq,       OverlapReceiv_25);
        ADD(OverlapReceiv_25,   EvProceedingReq,  DoProceedingReq, InCallProc_9);
        ADD(_AnyState_,         EvRelComplete,    DoRelCompete,    _DynamicState_);

        m_tDownstate = fsm.CloseTable();


        // ** Datastate ** 
        ADD(st_null,                PacketSetup,            RxSetupIn,      CallPresent_6); // may be Ovrlap_25          
        ADD(CallInitiated_1,        PacketProceeding,       RxProceeding,   OutCallProceeding_3);
        ADD(OverlapSending_2,       PacketProceeding,       RxProceeding,   OutCallProceeding_3);
        ADD(CallInitiated_1,        PacketSetupAck,         RxSetupAck,     OverlapSending_2);
        ADD(OutCallProceeding_3,    PacketAlerting,         RxAlerting,     CallDelivered_4);
        ADD(OverlapSending_2,       PacketAlerting,         RxAlerting,     CallDelivered_4);
        ADD(CallInitiated_1,        PacketAlerting,         RxAlerting,     _DynamicState_);
        ADD(CallDelivered_4,        PacketConnect,          RxConnect,      Active_10);
        ADD(OutCallProceeding_3,    PacketConnect,          RxConnect,      Active_10);
        ADD(OverlapSending_2,       PacketConnect,          RxConnect,      Active_10);
        ADD(OverlapSending_2,       PacketProgress,         RxProgress,     _SameState_);
        ADD(OutCallProceeding_3,    PacketProgress,         RxProgress,     _SameState_);
        ADD(CallDelivered_4,        PacketProgress,         RxProgress,     _SameState_);
        ADD(OverlapReceiv_25,       PacketInfo,             RxInfo,         OverlapReceiv_25);
        ADD(ConnectReq_8,           PacketConnectAck,       RxConnectAck,   Active_10);
        ADD(_AnyState_,             PacketRelease,          RxRelease,      _DynamicState_); //19,12,11,6,0
        ADD(_AnyState_,             PacketDisc,             RxDisconnect,   _DynamicState_);
        ADD(_AnyState_,             PacketReleaseComplete,  RxRelComplete,  _DynamicState_);

        m_tDatastate = fsm.CloseTable();


        // ** Expire Timer **
        ADD(CallInitiated_1,     EvT303,    ExpireT303,     _DynamicState_);
        ADD(DiscRequested_11,    EvT305,    ExpireT305,     ReleaseReq_19);
        ADD(ReleaseReq_19,       EvT308,    ExpireT308,     _DynamicState_);
        ADD(OutCallProceeding_3, EvT310,    ExpireT310,     DiscRequested_11);
        ADD(OverlapSending_2,    EvT304,    ExpireT304,     DiscRequested_11);
        ADD(ConnectReq_8,        EvT313,    ExpireT313,     DiscRequested_11);

        m_tExpireTimer = fsm.CloseTable();


        #undef ADD
    }

    // -------------------------------------------------

    void L3CallFsm::InitFsmTableNetwork( Fsm &fsm )
    {
        #define ADD UTILS_FSM_ADD

        // ** Downstate **
        ADD(st_null,                EvSetupReq,         DoSetupReq,         CallPresent_6);
        ADD(CallInitiated_1,        EvProceedingReq,    DoProceedingReq,    OutCallProceeding_3);
        ADD(OverlapSending_2,       EvProceedingReq,    DoProceedingReq,    OutCallProceeding_3);
        ADD(OutCallProceeding_3,    EvAlertingReq,      DoAlertingReq,      CallDelivered_4);
        ADD(CallDelivered_4,        EvSetupRsp,         DoSetupResponse,    Active_10);
        ADD(OutCallProceeding_3,    EvSetupRsp,         DoSetupResponse,    Active_10);
        ADD(ConnectReq_8,           EvSetupComplete,    DoSetupComplete,    Active_10);
        ADD(DiscRequested_11,       EvReleaseReq,       DoReleaseReq,       ReleaseReq_19);
        ADD(_AnyState_,             EvDiscReq,          DoDiscReq,          DiscIndication_12);
        ADD(CallInitiated_1,        EvMoreInfoReq,      DoInfoReq,          OverlapSending_2);
        ADD(_AnyState_,             EvRelComplete,      DoRelCompete,       _DynamicState_);    // команда срочного завершения

        m_tDownstate = fsm.CloseTable();

        // ** Datastate ** 
        ADD(st_null,            PacketSetup,            RxSetupIn,          CallInitiated_1);
        ADD(Active_10,          PacketConnectAck,       RxConnectAck,       Active_10);
        ADD(_AnyState_,         PacketReleaseComplete,  RxRelComplete,      _DynamicState_);            
        ADD(_AnyState_,         PacketDisc,             RxDisconnect,       _DynamicState_);   // DiscRequested_11,CCRelReq
        ADD(CallPresent_6,      PacketProceeding,       RxProceeding,       InCallProc_9);
        ADD(OverlapReceiv_25,   PacketProceeding,       RxProceeding,       InCallProc_9);
        ADD(CallPresent_6,      PacketConnect,          RxConnect,          ConnectReq_8);
        ADD(CallReceive_7,      PacketConnect,          RxConnect,          ConnectReq_8);
        ADD(InCallProc_9,       PacketConnect,          RxConnect,          ConnectReq_8);
        ADD(OverlapReceiv_25,   PacketConnect,          RxConnect,          ConnectReq_8);
        ADD(CallPresent_6,      PacketAlerting,         RxAlerting,         CallReceive_7);
        ADD(InCallProc_9,       PacketAlerting,         RxAlerting,         CallReceive_7);
        ADD(OverlapReceiv_25,   PacketAlerting,         RxAlerting,         CallReceive_7);
        ADD(InCallProc_9,       PacketProgress,         RxProgress,         _SameState_);
        ADD(OverlapReceiv_25,   PacketProgress,         RxProgress,         _SameState_);
        ADD(_AnyState_,         PacketRelease,          RxRelease,          _DynamicState_);
        ADD(OverlapSending_2,   PacketInfo,             RxInfo,             OverlapSending_2);

        m_tDatastate = fsm.CloseTable();

        // ** Expire Timer **
        ADD(CallPresent_6,      EvT303,     ExpireT303,     _DynamicState_);
        ADD(ReleaseReq_19,      EvT308,     ExpireT308,     _DynamicState_);
        ADD(CallReceive_7,      EvT301,     ExpireT301,     CallReceive_7);
        ADD(OverlapReceiv_25,   EvT304,     ExpireT304,     OverlapReceiv_25);
        ADD(DiscIndication_12,  EvT305,     ExpireT305,     ReleaseReq_19);

        m_tExpireTimer = fsm.CloseTable();

        #undef ADD
    }


} // ISDN



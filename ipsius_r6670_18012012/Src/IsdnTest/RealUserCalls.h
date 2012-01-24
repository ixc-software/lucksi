#ifndef REALUSERCALLS_H
#define REALUSERCALLS_H

#include "DssUserCall.h"
#include "SimulLiveStatistic.h"

namespace IsdnTest
{
    using ObjLink::ObjLinkBinder;
    using ISDN::IDssCall;
    using boost::shared_ptr;
    using ISDN::SetBCannels;
    using ISDN::CallWarning;
    using ISDN::DssCause;    

    // обычный исходящий вызов, после установки соединения выполняет разрыв
    class TypicalOutCall
        : public OutgoingDssUserCall
    {   
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;
        
    public:

        TypicalOutCall(ISubsToUserCall& subs, bool useAlerting);
    };

    //-------------------------------------------------------------------------

    // обычный входящий вызов (принемает и ожидает разрыва)
    class TypicalInCall
        : public IncommingDssUserCall
    {    
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;        

    public:

        TypicalInCall(ISubsToUserCall& subs, bool useAlerting);
    };

    //=========================================================================

    // пара Out/In CreateConnect создает, но не разрушает соединение

    class OutCreateConnect
        : public OutgoingDssUserCall
    {
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;
              
    public:

        OutCreateConnect(ISubsToUserCall& subs);
    };

    //-------------------------------------------------------------------------

    class InCreateConnect
        : public IncommingDssUserCall
    {
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;               

    public:

        InCreateConnect(ISubsToUserCall& subs);
    };

    //=========================================================================

    // исх вызов с ожиданием отказа (нет свободных каналов)
    class OutCheckBusyMsg
        : public OutgoingDssUserCall
    {
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;              

    public:

        OutCheckBusyMsg(ISubsToUserCall& subs);
    };    

    //=========================================================================    

    // вызовы разрывающие соединение до завершения установки соединения
    class OutDroppedBeforeConnect
        : public OutgoingDssUserCall
    {
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;        
        iCore::MsgTimer m_tDropTimer;

        void OnDropTimer(iCore::MsgTimer*);

    public:

        OutDroppedBeforeConnect(ISubsToUserCall& subs);
    };

    //-------------------------------------------------------------------------

    class InDroppedBeforeConnect
        : public IncommingDssUserCall
    {
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;           

    public:

        InDroppedBeforeConnect(ISubsToUserCall& subs);
    };

    //=========================================================================

    class NObjSimulLiveStatistic;
    class NObjSimulLiveSettings;


    // вызовы удерживающие соединение не более указанного времени
    class OutTalkByTime
        : public OutgoingDssUserCall
    {
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;        
        iCore::MsgTimer m_tTalkTimer;
        int m_talkTime;
        SimulLiveStatistic& m_statistic;

        void OnDiscByTimer(iCore::MsgTimer*);

        bool TryStartTimer();

        void DrawStat();

    public:

        OutTalkByTime(ISubsToUserCall& subs, int talkTime, const NObjSimulLiveSettings& m_prof);
    };

    //-------------------------------------------------------------------------

    class InTalkByTime
        : public IncommingDssUserCall
    {        
        class InternalFsm;
        Logger m_logSession;
        shared_ptr<InternalFsm> m_fsm;        
        iCore::MsgTimer m_tTalkTimer;
        int m_talkTime;
        SimulLiveStatistic& m_statistic;

        void OnDiscByTimer(iCore::MsgTimer*);

        bool TryStartTimer();        

        void DrawStat();

    public:

        InTalkByTime(ISubsToUserCall& subs, int talkTime, const NObjSimulLiveSettings& m_prof);
    };


   
} // namespace IsdnTest

#endif

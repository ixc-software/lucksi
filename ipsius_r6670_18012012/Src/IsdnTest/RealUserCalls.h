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

    // ������� ��������� �����, ����� ��������� ���������� ��������� ������
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

    // ������� �������� ����� (��������� � ������� �������)
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

    // ���� Out/In CreateConnect �������, �� �� ��������� ����������

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

    // ��� ����� � ��������� ������ (��� ��������� �������)
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

    // ������ ����������� ���������� �� ���������� ��������� ����������
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


    // ������ ������������ ���������� �� ����� ���������� �������
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

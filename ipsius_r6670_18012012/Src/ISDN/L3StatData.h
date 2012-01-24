#ifndef L3STATDATA_H
#define L3STATDATA_H

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace ISDN
{
    using Platform::dword;    

    class L3StatData
    {
        dword m_txCount;
        dword m_rxCount;
        dword m_badRxCount;

        dword m_callCount; // ���������� ��������� �������        
        dword m_currentTalkCount; // ������� ���������� ����������
        dword m_talkCount; // ���������� ����������

        QDateTime m_startStat; // ����� ������ ������� ����������
        dword m_statAge; // ����������������� �������� ����� ����������

        QDateTime m_lastDssActivation;
        dword m_totalActiveTime;
        dword m_activateDssCount;

        QDateTime m_lastChangeTalkCount; // ������ ���������� ��������� ����� ����������
        dword m_totalTalkTime; // �������� ����� ���������� (msec)


        void Init()
        {
            m_txCount = 0;
            m_rxCount = 0;
            m_badRxCount = 0;

            m_callCount = 0;
            m_talkCount = 0;
            m_currentTalkCount = 0;

            m_startStat = QDateTime();
            m_statAge = 0;

            m_lastDssActivation = QDateTime();
            m_totalActiveTime = 0;
            m_activateDssCount = 0;

            m_lastChangeTalkCount = QDateTime();
            m_totalTalkTime = 0;          
        }        

        L3StatData()
        {
            Init();
        }        

        friend class L3StatCollector;

    // Read stat const members
    public:
    
        dword getCallCount() const
        {
            return m_callCount;
        }

        //... other getters

        std::string getAsString() const;


    };      

    //===================================================================================

    class L3StatCollector
    {
        L3StatData m_data;
        bool m_statActive; // ��������� ������� ����� ����������
        bool m_dssInActive; // ������� ��������� Dss

        void Init();

    public:

        void ChangeTalkCount(int count);

        static int EvalPeriodSec(const QDateTime& from, const QDateTime& to);

    public:

        L3StatCollector(bool statActive, bool dssActive = false);

    // Commands 
    public:

        void ActivateStat(bool dssActive);

        void DeactivateStat();

        void Clear();

    // Accumulate stat members 
    public:        

        void TxInc();

        void RxInc();

        void BadPackInc();

        void CallInc();

        void TalkInc();

        void TalkDec();

        void DssActivated();

        void DssDeactivated();

        L3StatData getData() const;

    };

} //namespace ISDN

#endif


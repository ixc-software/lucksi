#ifndef L2STATDATA_H
#define L2STATDATA_H
/*
    ����������� ����� ��� ������� ���������� IsdnL2
*/

#include "stdafx.h"
#include "Platform/PlatformTypes.h"

namespace ISDN
{    
    
    using Platform::dword;

    class L2StatData
    {
        friend class L2StatCollector;

        dword m_TxIcount; // ������� ������������ IPacket
        dword m_RxIcount; // ������� �������� IPacket
        dword m_TxUcount; // ������� ������������ IPacket
        dword m_RxUcount; // ������� �������� IPacket
        dword m_badCount; // ������� ����� �������
        dword m_TxAllPackCount; // ������� ���� �������� �������
        dword m_RxAllPackCount; // ������� ���� ������������ �������
        QDateTime m_timeAtStartSession; // ������ ��������� lapd        
        QDateTime m_timeAtStopSession; // ������ ��������� lapd        
        dword m_totalSessionTime;  // ����� ����� ���������� lapd 

        QDateTime m_timeAtActivation; // ������ ������ ����� ����������             
        dword m_statAge;                  

        void Init()
        {
            m_TxIcount = 0;
            m_RxIcount = 0;
            m_TxUcount = 0;
            m_RxUcount = 0;
            m_TxAllPackCount = 0;
            m_RxAllPackCount = 0;
            m_badCount = 0;
            m_timeAtStartSession = QDateTime();            
            m_totalSessionTime = 0;     
            m_timeAtActivation = QDateTime();
            m_statAge = 0;            
        }      

        L2StatData()
        {
            Init();
        }

    // const members
    public:

        int getTxICount() const
        {
            return m_TxIcount;
        }

        int getRxICount() const
        {
            return m_RxIcount;
        }

        int getTxUCount() const
        {
            return m_TxUcount;
        }

        int getRxUCount() const
        {
            return m_RxUcount;
        }

        int getAllTx() const
        {
            return m_TxAllPackCount;
        }

        int getAllRx() const
        {
            return m_RxAllPackCount;
        }        
        
        int getStatAge() const
        {
            return m_statAge;
        }

        int getTotalLapdActiveTime() const
        {            
            return m_totalSessionTime;         
        }

        std::string getAsString() const;
        
    };    

    //=============================================================================

    class L2StatCollector
    {
        L2StatData m_data;
        bool m_sessionStarted;
        bool m_active; 

        static dword EvalPeriodSec(const QDateTime& current, const QDateTime& prev);

        void Init();

    public:

        L2StatCollector(bool active, bool lapdActive = false);

        void IPackTxInc();

        void UPackTxInc();

        void IPackRxInc();

        void UPackRxInc();

        void AllTxInc();

        void AllRxInc();

        void BadPackInc();

        void ActivateMoment();

        void DeactivateMoment();

        void ActivateStat(bool lapdActive);

        void Clear();

        L2StatData getData() const;

    };
    
} //namespace ISDN

#endif

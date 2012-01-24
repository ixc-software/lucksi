#ifndef NOBJLAPDTESTSETTINGS_H
#define NOBJLAPDTESTSETTINGS_H

#include "DRI/INonCreatable.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "NObjLapdTraceOption.h"

namespace IsdnTest
{
    class NObjLapdTestSettings
        : public Domain::NamedObject,
        public DRI::INonCreatable
    {
        Q_OBJECT
    public:
        
        int m_MaxDurationMsec;          // General timeout of test running
        int m_MaxTestTimeout;           // Timeout for DomainTestRunner

        // tracing        
        bool m_TracedTest;                // trace-on in test if true
        NObjLapdTraceOption *const m_pL2TraceOption; // trace option for L2Profile        
        bool m_UseTimeStamp;              // use timestamp in trace

        // data exchange
        int m_DropRate;                   // percent of drop packet in DropMode
        int m_DataExchangeCycle;          // number of cycles of DataExchaneTest

        int m_MaxIPackets;                // max number of IPacket send in session
        int m_MaxUPackets;                // max number of UPacket send in session
        int m_MaxIPacketsInDropMode;      // max number of IPacket send in session in DropMode
        
        //bool m_ThrowThreadException;      // throw test exception from thread

    public:

        NObjLapdTestSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent),
            m_pL2TraceOption(new NObjLapdTraceOption(this, "L2TraceOption"))
        {
            m_MaxDurationMsec = 300 * 1000;  /*300 000 = 5min*/            
            m_MaxTestTimeout = m_MaxDurationMsec + 20000;
            m_TracedTest = false;            
            m_UseTimeStamp = true;
            m_DropRate = 20;
            m_DataExchangeCycle = 5;
            m_MaxIPackets = 12;
            m_MaxUPackets = 12;
            m_MaxIPacketsInDropMode = 12;            
            //m_ThrowThreadException = false;
        }

        Q_PROPERTY(int MaxDurationMsec READ m_MaxDurationMsec WRITE m_MaxDurationMsec);
        Q_PROPERTY(int MaxTestTimeout READ m_MaxTestTimeout WRITE m_MaxTestTimeout);
        Q_PROPERTY(bool TraceTest READ m_TracedTest WRITE m_TracedTest);
        Q_PROPERTY(int DropRate READ m_DropRate WRITE m_DropRate);
        Q_PROPERTY(int DataExchangeCycle READ m_DataExchangeCycle WRITE m_DataExchangeCycle);
        Q_PROPERTY(int MaxIPackets READ m_MaxIPackets WRITE m_MaxIPackets);
        Q_PROPERTY(int MaxUPackets READ m_MaxUPackets WRITE m_MaxUPackets);
        Q_PROPERTY(int MaxIPacketsInDropMode READ m_MaxIPacketsInDropMode WRITE m_MaxIPacketsInDropMode);        
        //Q_PROPERTY(bool ThrowThreadException READ m_ThrowThreadException WRITE m_ThrowThreadException);

        void ValidateFields() 
        {            
            /*if (MaxDurationMsec >= MaxTestTimeout)
                ESS_THROW_MSG(WrongParametrs, "Must be: MaxDurationMsec < MaxTestTimeout" );
            if (
                m_DropRate < 0 &&
                m_DataExchangeCycle < 0 &&
                m_MaxIPackets < 0 &&
                m_MaxUPackets < 0 &&
                m_MaxIPacketsInDropMode < 0
                ) ESS_THROW_MSG()*/
        }
    
    };
} // namespace IsdnTest

#endif

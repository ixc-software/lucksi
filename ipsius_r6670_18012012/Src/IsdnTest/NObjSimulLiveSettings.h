#ifndef NOBJSIMULLIVESETTINGS_H
#define NOBJSIMULLIVESETTINGS_H

#include "NObjDssTestSettings.h"
#include "SimulLiveStatistic.h"

namespace IsdnTest
{
    class NObjSimulLiveSettings
        : public Domain::NamedObject,
        public DRI::INonCreatable
    {
        Q_OBJECT  
    public:
        NObjDssTestSettings *const m_pGeneralSettings;
        
        int m_PrcentDropedCallInConnection; // - percent of calls dropped in connection process        
        int m_TestDuration; // test duration (msec) 

        int m_MinCallInterval; // min. interval of generating new call
        int m_MaxCallInterval; // max. interval of generating new call

        int m_MinTalkTime; // min. interval of call in talk state
        int m_MaxTalkTime; // max. interval of call in talk state

        bool m_OutStatistic; // if true output statistic to console when test finished
        mutable SimulLiveStatistic Statistic;

    public:
        NObjSimulLiveSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent),
            m_pGeneralSettings(new NObjDssTestSettings(this, "GeneralProfile"))
        {
            // default parameters:
            m_PrcentDropedCallInConnection = 10;            
            m_TestDuration = 1 * 10 * 1000;         
            m_MinCallInterval = 90;
            m_MaxCallInterval = 180;
            m_MinTalkTime = 3300;
            m_MaxTalkTime = 4350;
            m_OutStatistic = true;
            m_pGeneralSettings->m_MaxTestTimeout = 3 * 1000 + m_TestDuration; // -1
        }        
    
        Q_PROPERTY(int PrcentDropedCallInConnection READ m_PrcentDropedCallInConnection WRITE m_PrcentDropedCallInConnection);
        Q_PROPERTY(int TestDuration READ m_TestDuration WRITE m_TestDuration);
        Q_PROPERTY(int MinCallInterval READ m_MinCallInterval WRITE m_MinCallInterval);
        Q_PROPERTY(int MaxCallInterval READ m_MaxCallInterval WRITE m_MaxCallInterval);
        Q_PROPERTY(int MinTalkTime READ m_MinTalkTime WRITE m_MinTalkTime);
        Q_PROPERTY(int MaxTalkTime READ m_MaxTalkTime WRITE m_MaxTalkTime);
        Q_PROPERTY(int OutStatistic READ m_OutStatistic WRITE m_OutStatistic);
        
    };
   
} // namespace IsdnTest

#endif

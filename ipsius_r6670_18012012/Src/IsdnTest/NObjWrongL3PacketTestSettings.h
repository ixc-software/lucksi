#ifndef NOBJWRONGL3PACKETTESTSETTINGS_H
#define NOBJWRONGL3PACKETTESTSETTINGS_H

#include "DRI/INonCreatable.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "RandomPatternGen.h"
#include "NObjRndPatternGenSettings.h"

namespace IsdnTest
{  
    class NObjWrongL3PacketTestSettings : 
        public Domain::NamedObject,
        public DRI::INonCreatable        
    {
        Q_OBJECT;        

    public:

        int m_MinLen; // минимальная длина случайного пакета
        int m_MaxLen; // максимальная длина случайного пакета
        int m_rndPackPercent; // процент пакетов сгенерированных как набор случайных байтов
        int m_Duration; // продолжительность тестирования, если -1, то неограничено

        int m_verboseTimeSec; // интервал вывода статистики

        NObjRndPatternGenSettings PatternProf;        

        NObjWrongL3PacketTestSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent),
            m_MinLen(3),
            m_MaxLen(524),
            m_rndPackPercent(50),
            m_Duration(-1),            
            m_verboseTimeSec(10),
            PatternProf(this, "RndPatternGenProf")
        {        
        }

        Q_PROPERTY (int MinLenRndPacket READ m_MinLen);
        Q_PROPERTY (int MaxLenRndPacket READ m_MaxLen);
        Q_PROPERTY (int DurationSec READ m_Duration WRITE m_Duration);
        Q_PROPERTY (int RndPackPercent READ m_rndPackPercent WRITE m_rndPackPercent);
        Q_PROPERTY (int VerboseTimeSec READ m_verboseTimeSec WRITE m_verboseTimeSec);
        Q_INVOKABLE void LenRange(int from, int to)
        {
            if (from > to) ThrowRuntimeException("Wrong range");
            m_MinLen = from;
            m_MaxLen = to;
        }                     
    };

} // namespace IsdnTest

#endif

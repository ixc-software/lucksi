#ifndef DSSTIMERSPROF_H
#define DSSTIMERSPROF_H

#include "stdafx.h"
//#include "IsdnUtils.h"
#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"

namespace ISDN
{
    using Platform::dword;

    struct DssTimerValue
    {
        dword m_interval;
        int m_maxRepeat; // -1 не ограниченно
        bool m_inited;
        DssTimerValue(dword interval, int maxRepeat = 1)
            : m_interval(interval), m_maxRepeat(maxRepeat), m_inited(true) {} 
        DssTimerValue()
            : m_interval(-1), m_maxRepeat(0), m_inited(false) {}         
        QString ToString() const;
    };
   
    // Профайл настройки таймеров Dss - системы
    // **поля не представленных(выключенных таймеров) не инициализируются!
    struct DssTimersProf
    {        
        DssTimerValue T301;
        DssTimerValue T302;
        DssTimerValue T303;
        DssTimerValue T304;
        DssTimerValue T305;
        DssTimerValue T306;
        DssTimerValue T307;
        DssTimerValue T308;
        DssTimerValue T309;
        DssTimerValue T310;
        DssTimerValue T312;
        DssTimerValue T313;
        DssTimerValue T314;
        DssTimerValue T316;
        DssTimerValue T317;
        DssTimerValue T318;
        DssTimerValue T319;
        DssTimerValue T320;
        DssTimerValue T321;
        DssTimerValue T322;
        

        DssTimerValue tReActivateReqL3; // таймер перезапроса активации для перезагруженного IsdnL3
        DssTimerValue tWaitFreeWinInd;  // таймер ожидания индикации доставки всех пакетов вторым уровнем
        DssTimerValue tProcessErrorF;   // таймер-счетчик ошибок F от 2 уровня     

        bool Validate();        

        DssTimersProf() : m_isInited(false) {};

        void InitNetDefault();
        void InitUserDefault(); 

    private:
        bool m_isInited;
    };
    
} // namespace ISDN

#endif


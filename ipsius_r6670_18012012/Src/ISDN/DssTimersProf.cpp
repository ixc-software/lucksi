#include "stdafx.h"
#include "DssTimersProf.h"

namespace ISDN
{
    void DssTimersProf::InitNetDefault()
    {        
            T301 = 200*1000;                    // >= 3min
            T302 = 12*1000;                     // 10 - 15s
            T303 = DssTimerValue(4*1000, 2);    // 4s
            T304 = (20*1000);                   // 20s
            T305 = (30*1000);                   // 30s
            T306 = (30*1000);                   // 30s
            T307 = (180*1000);                  // 180s

            T308 = DssTimerValue(4*1000, 2);    // 4s N-?            

            //T309 = (90*1000);                   // 90s
            T310 = (10*1000);                   // 10s
            T312 = (2*1000 + T303.m_interval);  // T303 + 2s
            //T313=(*1000);                     // not present
            T314 = (4*1000);                    // 4s
            T316 = DssTimerValue(120*1000, -1); // 2min N-?

            T317 = (100*1000);                  // impl-depended, < T316            
            ESS_ASSERT(T316.m_interval > T317.m_interval);

            T320 = (30*1000);                   // 30s
            T321 = (30*1000);                   // 30s
            T322 = DssTimerValue(4*1000, 10);       
            //T318=(*1000);                     // not present
            //T319=(*1000);                     // not present

            tReActivateReqL3 = (1500);
            tWaitFreeWinInd = (10*1000);
            tProcessErrorF = DssTimerValue(1020, 20);            

            m_isInited = true;
    }        

    void DssTimersProf::InitUserDefault()
    {          
            T301 = (200*1000);                  // >= 3min
            T302 = (15*1000);                   // 15s
            T303 = DssTimerValue(4*1000, 2);    // 4s
            T304 = (30*1000);                   // 30s
            T305 = (30*1000);                   // 30s
            //T306(*1000),                      // not present
            //T307(*1000),                      // not present

            T308 = DssTimerValue(4*1000, 2);    // 4s       

            //T309 = (90*1000);                   // 90s
            T310 = (60*1000);                   // 30 - 120s
            //T312(*1000);                      // not present
            //T313 = (4*1000);                    // 4s
            T314 = (4*1000);                    // 4s
            T316 = DssTimerValue(120*1000, -1); // 2min

            T317 = (100*1000);                  // impl-depended, < T316
            ESS_ASSERT(T316.m_interval > T317.m_interval);

            T318 = (4*1000);                    // 4s
            T319 = (4*1000);                    // 4s
            //T320=(*1000);                     // not present
            T321 = (30*1000);                   // 30s
            T322 = DssTimerValue(4*1000,10);    // 4s       

            tReActivateReqL3 = (800);
            tWaitFreeWinInd = (10*1000);
            tProcessErrorF = DssTimerValue(1020, 20);

            m_isInited = true;
    }

    bool DssTimersProf::Validate()
    {
        if (!m_isInited) return false;
        if (tProcessErrorF.m_inited && tProcessErrorF.m_interval != 1020) return false;
        if (T316.m_inited && T316.m_interval <= T317.m_interval) return false;
        // todo: other

        return true;        
    }    
   

    QString DssTimerValue::ToString() const
    {
        if (!m_inited) return "NotUsed";
        return QString("%1/%2 (sec/repeat)").arg(m_interval/1000).arg(m_maxRepeat);
    }
} // namespace ISDN


#ifndef __PSBCOFIDECINIT__
#define __PSBCOFIDECINIT__

#include <cdefBF537.h>
#include <ccblkfn.h>

#include "Utils/ErrorsSubsystem.h"

namespace PsbCofidec
{

    class PsbCofidecInit
    {

        static bool& GetInitDone()
        {
            static bool initDone = false;
            return initDone;
        }
                
    public:

        static bool InitDone()
        {
            return GetInitDone();
        }

        static bool InitSync()
        {
            ESS_ASSERT(!InitDone());

            *pTIMER_DISABLE		|= 0x0030;
            *pPORTFIO_INEN		|= 0x8000;
            *pPORTF_FER	|= 0x8030;

            *pTIMER4_CONFIG		= 0x00AD;   // 2048 KHz for TimeShift of TDM_CLK
            *pTIMER4_PERIOD		= 8;
            *pTIMER4_WIDTH		= 4;

            *pTIMER5_CONFIG		= 0x00AD;  //AOZ_Frame
            *pTIMER5_PERIOD		= 2048;
            *pTIMER5_WIDTH		= 8;

            *pTIMER_ENABLE		|= 0x0030;        						

            GetInitDone() = true;

            return true;
        }        
        
    };
    
    
}  // namespace PsbCofidec


#endif

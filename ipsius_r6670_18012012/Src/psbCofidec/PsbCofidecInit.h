#ifndef __PSBCOFIDECINIT__
#define __PSBCOFIDECINIT__

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

            // nothing 

            GetInitDone() = true;

            return true;
        }        
        
    };
    
    
}  // namespace PsbCofidec


#endif

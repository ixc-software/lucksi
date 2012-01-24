#ifndef __AOZINIT__
#define __AOZINIT__

namespace DevIpTdm
{

    class AozInit
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

        static void LockTimers()
        {
            // nothing
            // ... 
        }
    };

        
}  // namespace DevIpTdm


#endif

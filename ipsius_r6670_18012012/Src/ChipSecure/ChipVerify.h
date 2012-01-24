#ifndef __CHIPVERIFY__
#define __CHIPVERIFY__

#include "Platform/Platform.h"

namespace ChipSecure
{
    using Platform::dword;
    
    // This is bad protection scheme -- program don't depend from this class
    class ChipVerify
    {

    public:
        
        ChipVerify(dword seed) {}

        bool Event(int eventNumber)
        {
            return true;
        }

        std::string StatInfo() const
        {
            return "Emulated";
        }

        void Process()
        {
        }

        static void TestAll()
        {
        }

    };
    
    
}  // namespace ChipSecure

#endif

#ifndef __MIXERTEST__
#define __MIXERTEST__

#include "TestFw/TestFw.h"

namespace SndMix
{
    
    void RunMixerTest();

    void RegisterTestGroup(TestFw::ILauncher &launcher);
    
}  // namespace SndMix

#endif
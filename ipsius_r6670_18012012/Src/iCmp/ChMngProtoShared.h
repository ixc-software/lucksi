#ifndef __CHMNGPROTOSHARED__
#define __CHMNGPROTOSHARED__

namespace iCmp
{
    
    // for PcCmdSetFreqRecvMode
    enum FreqRecvMode  
    {
        FrOn,             // On  now
        FrOff,            // Off now
        FrAutoOn,         // Off now + On if line goes busy
        FrAutoOnOff,      // line busy -> on/off
    };
        
}  // namespace iCmp

#endif

#ifndef __MIXERERROR__
#define __MIXERERROR__

#include "Utils/ErrorsSubsystem.h"

namespace SndMix
{
    
    struct Error
    {
        ESS_TYPEDEF(Base);
        
        ESS_TYPEDEF_FULL(CantFindCodec, Base);
        ESS_TYPEDEF_FULL(G711DontHaveParams, Base);
        ESS_TYPEDEF_FULL(BadHostPort, Base);
        ESS_TYPEDEF_FULL(BadDeviceName, Base);
        ESS_TYPEDEF_FULL(PointNotInConf, Base);
        ESS_TYPEDEF_FULL(PointAlreadyInConf, Base);
        ESS_TYPEDEF_FULL(BadPointModeForGen, Base);
        ESS_TYPEDEF_FULL(BadGenProfile, Base);
        ESS_TYPEDEF_FULL(HandleLimit, Base);
        ESS_TYPEDEF_FULL(HandleNotEmpty, Base);
                
        ESS_TYPEDEF_FULL(DublicateTdmPoint, Base);
        ESS_TYPEDEF_FULL(BadPointHandle, Base);
        ESS_TYPEDEF_FULL(BadPointType, Base);        
        ESS_TYPEDEF_FULL(BadHandle, Base);
        ESS_TYPEDEF_FULL(SetModeForAutoConf, Base);
        
        ESS_TYPEDEF_FULL(MixerCoreBadMode, Base);

        ESS_TYPEDEF_FULL(MixerModeParseError, Base);
                                        
    };
    
}  // namespace SndMix

#endif

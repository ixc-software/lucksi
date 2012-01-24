#ifndef __SBPEXCEPTION__
#define __SBPEXCEPTION__

#include "Utils/ErrorsSubsystem.h"

namespace SBProto
{

    // Frame/SbpPackInfo exeptions hierarchy
    ESS_TYPEDEF(SpbException);

    ESS_TYPEDEF_FULL(BadFieldIndex,     SpbException);
    ESS_TYPEDEF_FULL(BadFieldType,      SpbException);
    ESS_TYPEDEF_FULL(BufferTooSmall,    SpbException);
    ESS_TYPEDEF_FULL(WStringOddSize,    SpbException);
       
}  // namespace SBProto

#endif

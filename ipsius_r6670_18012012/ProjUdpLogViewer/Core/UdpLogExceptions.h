#ifndef __LOGVIEWEXCEPTIONS__
#define __LOGVIEWEXCEPTIONS__

#include "Utils/ErrorsSubsystem.h"

namespace Ulv
{
    ESS_TYPEDEF(UdpLogException);

    ESS_TYPEDEF_FULL(LogRecordParsingFailed, UdpLogException);
    // ESS_TYPEDEF_FULL(OddWStringBytesCount, UdpLogException);


} // namespace Ulv

#endif

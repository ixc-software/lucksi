#ifndef DSSINCALLPARAMS_H
#define DSSINCALLPARAMS_H

#include "DssCallParams.h"
#include "IIsdnAlloc.h"

namespace ISDN
{
    class PacketSetup;

    // параметры входящего вызова
    class DssInCallParams : public DssCallParams
    {
        DssInCallParams(const PacketSetup& pack, const BChannelsWrapper& captured);

    public:

        static DssInCallParams* Create(IIsdnAlloc& alloc, const PacketSetup& pack, const BChannelsWrapper& captured);
    };

} // ISDN

#endif


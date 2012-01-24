#ifndef __UDPLOGFILTER__
#define __UDPLOGFILTER__

#include "UlvTypes.h"
#include "UdpLogRecordViewRange.h"

namespace Ulv
{
    class UdpLogFilter
    {
        static bool FindAllOf(int filterPosOffset, // using in ranges
                              const QStringList &expressions, Qt::CaseSensitivity cs,
                              const QString &data,
                              UdpLogRecordViewRangesList &addToFilterRanges);

        static bool FoundExp(int filterPosOffset, // using in ranges
                             const QString &oneExp, Qt::CaseSensitivity cs,
                             const QString &data,
                             UdpLogRecordViewRangesList &addToFilterRanges);

    public:
        static bool Match(const FilterExp &exp,
                          const QString &rec, int recStartPos,  bool recSrcSelected,
                          UdpLogRecordViewRangesList &addToRanges);
    };

} // namespace Ulv

#endif

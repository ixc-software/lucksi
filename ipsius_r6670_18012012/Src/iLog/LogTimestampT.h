#ifndef __LOGTIMESTAMPT__
#define __LOGTIMESTAMPT__

namespace iLog
{

    // тип, хран€щий временнќй слепок
    class LogTimestampT
    {
    public:
        enum CaptureModeType {};

        LogTimestampT(const CaptureModeType &mode);

        bool IsEmpty() const;
        int Compare(const LogTimestampT &other) const;
        
        static CaptureModeType DefaultCaptureMode();
        static CaptureModeType DefaultSystemCaptureMode();
        static LogTimestampT   EmptyTimestamp();

    };

}  // namespace iLog

#endif


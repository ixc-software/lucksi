#ifndef __LOGTIMESTAMPT__
#define __LOGTIMESTAMPT__

namespace iLog
{

    // ���, �������� ��������� ������
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


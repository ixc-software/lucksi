#ifndef __UDPLOGRECORDINGTOFILE__
#define __UDPLOGRECORDINGTOFILE__


#include "IUdpLogRuntimeRecording.h"
#include "UlvTypes.h"
namespace Ulv
{
    class UdpLogRecordingToFile  :
        public IUdpLogRuntimeRecording
    {
        RecordViewSett m_format;
        QString m_file;

    // IUdpLogRuntimeRecording impl
    private:
        void Add(const QString &formattedData);
        const RecordViewSett& Format() const { return m_format; }
        QString DestDescription() const;
        ddword RecordedSize() const;

    public:
        UdpLogRecordingToFile(); // can throw
    };

} // namespace Ulv

#endif

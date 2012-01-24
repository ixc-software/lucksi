#ifndef UDPLOGDBPARAMS_H
#define UDPLOGDBPARAMS_H

#include "IUdpLogRuntimeRecording.h"
#include "IUdpPackParser.h"


namespace Ulv
{
    class UdpLogDBParams
    {
        int m_recordsCount;
        int m_dropRecordsCount;

        boost::shared_ptr<IUdpPackParser> m_parser;

        boost::shared_ptr<IUdpLogRuntimeRecording> m_recorder;
        int m_recIntervalMs;
        int m_recRecordCount;

    public:
        UdpLogDBParams();

        void RecordCount(int count);
        int RecordCount() const { return m_recordsCount; }
        int DropRecordCount() const { return m_dropRecordsCount; }

        void EnableRecording(bool state);
        bool RecordingEnabled() const;

        void RecordingIntervalMs(int val) { m_recIntervalMs = val; }
        int RecordingIntervalMs() const { return m_recIntervalMs; }

        void RecordingRecordCount(int val) { m_recRecordCount = val; }
        int RecordingRecordCount() const { return m_recRecordCount; }

        QString RecordingTo() const;

        IUdpLogRuntimeRecording& Recorder();
        IUdpPackParser& Parser();
        const IUdpPackParser& Parser() const;

        static int DefaultRecordCount();
        static int DefaultRecordingIntervalMs();
        static int DefaultRecordingRecordCount();
    };

} // namespace Ulv


#endif // UDPLOGDBPARAMS_H

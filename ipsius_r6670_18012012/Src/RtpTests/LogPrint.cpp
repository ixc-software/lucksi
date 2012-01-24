#include "stdafx.h"

#include "iLog/LogWrapperInterfaces.h"
#include "Utils/ExeName.h"
#include "LogPrint.h"

namespace
{
    static QString DoFormatRecord(iLogQStr::LogRecord &record, iLogQStr::LogStore &logStore)
    {            
        QDateTime dt = record.getTimestamp().get();
        QString time = "<empty>";
        if (!dt.isNull()) time = dt.toString("hh:mm:ss.zzz");

        QString data = record.getData();
        QString session = logStore.ResolveSessionTag( record.getSessionTag() );
        QString recordKind = record.getKindValue();

        return QString("%1 [%2] [%3] %4\n").arg(time, -12).arg(session, -16).arg(recordKind, -20).arg(data); 
    }

    // -----------------------------------------------

    // implementation iLogW::ILogSessionHandler 
    class LogHandler : public iLogW::ILogSessionHandler
    {
        iLogQStr::LogStore &m_logStore;
        bool m_printToCout; 
        bool m_printToFile;
        bool m_delaySync;
        std::ofstream m_file;

        bool DoSyncWithStore(iLogQStr::LogRecord &record) // override
        {
            if (m_printToCout)
                std::cout << DoFormatRecord(record, m_logStore).toStdString() << std::endl;

            if (m_printToFile)
                m_file << DoFormatRecord(record, m_logStore).toStdString();

            return (m_delaySync) ? false : true;
        }

    public:

        LogHandler(const RtpTests::LogPrint &logPrint, bool delaySync) 
            : m_logStore(logPrint.getLogStore().getSessionInterface().getLogStore()),
            m_printToCout(logPrint.isPrintToCout()),
            m_printToFile(logPrint.getFileName().empty() == false),
            m_delaySync(delaySync)
        {
            if (m_printToFile)
            {
                QString fullPath = Utils::ExeName::GetExeDir() + 
                                   QString(logPrint.getFileName().c_str());
                m_file.open(fullPath.toStdString().c_str() ,std::ios::out);
            }
        }
    };

} // namespace

namespace RtpTests
{

    boost::shared_ptr<iLogW::ILogSessionHandler> 
        LogPrint::CreateLogHandler(const LogPrint &logPrint, bool delaySync)
    {
        return boost::shared_ptr<iLogW::ILogSessionHandler>(
            new LogHandler(logPrint, delaySync));
    }

} // namespace RtpTests

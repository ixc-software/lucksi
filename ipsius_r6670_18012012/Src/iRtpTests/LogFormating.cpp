//#include "stdafx.h"
//#include "LogFormating.h"
//#include "LogOutSettings.h"
//#include "Utils/ExeName.h"
//#include "iLog/LogWrapperInterfaces.h"
//
//namespace iRtpTests
//{
//    namespace
//    {
//        static QString DoFormatRecord(iLogQStr::LogRecord &record, iLogQStr::LogStore &logStore)
//        {                
//            QString data = record.getData();
//            QString session = logStore.ResolveSessionTag( record.getSessionTag() );
//            QString recordKind = record.getKindValue();
//
//            QDateTime dt = record.getTimestamp().get();            
//            if (!dt.isNull())
//            {
//                QString time = dt.toString("hh:mm:ss:zzz");
//                return QString("\n[time: %1]\n[from: %2]\n[Kind: %3] \n%4\n").arg(time, -10).arg(session, -50).arg(recordKind, -12).arg(data); 
//            }
//            else
//                return QString("\n[from: %1]\n[Kind: %2] \n%3\n").arg(session, -50).arg(recordKind, -12).arg(data);            
//        }
//    }
//
//
//
//    class FileSaveSelectAndFormat::Impl : 
//        public iLogW::ILogSaveToFile,
//        public iLogW::ILogRecordFormater
//    {
//        iLogQStr::LogStore &m_logStore;
//        iLog::LogSelector<iLogQStr::LogRecord> m_selector;
//
//        // ILogRecordFormater impl
//    private:
//
//        QString FormatRecord(iLogQStr::LogRecord &record)  // override
//        {            
//            return DoFormatRecord(record, m_logStore); 
//        }
//
//        // iLogW::ILogSaveToFile
//    public:
//
//        iLog::ILogSelector<iLogQStr::LogRecord>& getSelector()
//        {
//            return m_selector;
//        }
//
//        ILogRecordFormater& getFormater()
//        {
//            return *this;
//        }
//
//    public:
//
//        Impl(iLogW::LogStore &store) 
//            : m_logStore(store.getSessionInterface().getLogStore())
//        {}        
//
//
//    };
//
//    FileSaveSelectAndFormat::FileSaveSelectAndFormat(iLogW::LogStore &store) 
//        : m_pImpl( new FileSaveSelectAndFormat::Impl(store) )
//    {}
//
//    iLogW::ILogSaveToFile& FileSaveSelectAndFormat::getSaverInterface()
//    {
//        return *m_pImpl;
//    }
//
//    //=====================================================================================================
//
//    class SessionHandler::Impl : public iLogW::ILogSessionHandler
//    {
//        iLogQStr::LogStore &m_logStore;
//        bool m_printToCout; 
//        bool m_delaySync;
//        std::ofstream m_fileOut;
//        bool m_toFile;
//
//        bool DoSyncWithStore(iLogQStr::LogRecord &record) // override
//        {
//            if (m_printToCout)
//            {
//                std::cout << DoFormatRecord(record, m_logStore).toStdString();               
//            }
//
//            if(m_toFile)
//                m_fileOut << DoFormatRecord(record, m_logStore).toStdString();               
//
//            return (m_delaySync) ? false : true;
//        }
//
//    public:
//
//        Impl(const LogOutSettings& prof, bool delaySync)
//            : m_logStore(prof.getISessionToStore().getLogStore()),
//            m_printToCout(prof.m_writeToConsole),
//            m_delaySync(delaySync),
//            m_toFile(!prof.m_writeToFileAfter && !prof.m_file.isEmpty() )
//        {
//            if(m_toFile)
//                m_fileOut.open( (Utils::ExeName::GetExeDir() + prof.m_file).toStdString().c_str(), std::ios::out);
//        }      
//    };
//
//
//
//    SessionHandler::SessionHandler(const LogOutSettings& prof, bool delaySync)
//        : m_pImpl( new SessionHandler::Impl(prof, delaySync) )
//    {}
//
//    iLogW::ILogSessionHandler* SessionHandler::getHandlerInterface()
//    {
//        return m_pImpl.get();
//    }
//}

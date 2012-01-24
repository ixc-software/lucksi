//#ifndef LOGFORMATING_H
//#define LOGFORMATING_H
//
//#include "iLog/LogWrapper.h"
//
//namespace iRtpTests
//{
//    // форматирование вывода трассировки, запись в файл
//    class FileSaveSelectAndFormat
//    {
//        class Impl;
//        boost::shared_ptr<Impl> m_pImpl;
//
//    public:
//
//        FileSaveSelectAndFormat(iLogW::LogStore &store);
//
//        iLogW::ILogSaveToFile& getSaverInterface();        
//
//    };
//
//    // ------------------------------------------------------------
//
//    class LogOutSettings;
//
//    class SessionHandler
//    {
//        class Impl;
//        boost::shared_ptr<Impl> m_pImpl;
//
//    public:
//
//        SessionHandler(const LogOutSettings& prof, bool delaySync = true);        
//
//        iLogW::ILogSessionHandler* getHandlerInterface();
//
//    };
//} // namespace iRtp
//
//#endif

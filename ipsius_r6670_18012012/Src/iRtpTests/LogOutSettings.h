#ifndef LOGOUTSETTINGS_H
#define LOGOUTSETTINGS_H

#include "stdafx.h"
#include "iLog/LogWrapper.h"

namespace iRtpTests
{
    class iLogW::ILogSessionToLogStore;

    class LogOutSettings : public boost::noncopyable
    {

    private:

        mutable iLogW::LogStore& m_store;    
        
        void InitDefault()
        {
            m_writeToConsole = true;
            m_writeToFileAfter = false;
            m_file = "iRtpTest.log";
        }        

    public:

        explicit LogOutSettings(iLogW::LogStore& store)
            : m_store(store)
        {
            InitDefault();
        }

        iLogW::ILogSessionToLogStore& getISessionToStore() const
        {
            return m_store.getSessionInterface();
        }

        bool m_writeToConsole; // выводить в консоль
        bool m_writeToFileAfter; // писать в файл после завершения теста, иначе - одновременно
        QString m_file; // имя файла, если нет то в файл не писать
        
        
    };
} // namespace iRtpTests

#endif

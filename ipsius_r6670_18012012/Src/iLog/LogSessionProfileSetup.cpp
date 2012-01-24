#include "stdafx.h"
#include "Utils/ExeName.h"
#include "LogWrapper.h"
#include "LogWrapperLibrary.h"
#include "LogSessionSettings.h"
#include "LogSessionProfileSetup.h"

namespace iLogW
{
    class LogSessionProfileSetup::Impl : boost::noncopyable
    {
    public:        

        Impl(const LogSessionSettings &settings, 
            boost::shared_ptr<ILogRecordFormater> formater) :
            m_sync(settings.getCountSyncroToStore(), settings.getTimeoutSyncroToStore()),
            m_profile(settings.isTimestampUsed(), &m_output, &m_sync)
        {
            if (settings.isOutputToCoutOn())
            {
                m_coutStream.reset(new LogCoutStream(formater));
                m_output.Register(m_coutStream.get());
            }
            if (settings.isOutputToFileOn())
            {
                m_fileStream.reset(
                    new LogFileStream(Utils::ExeName::GetExeDir() + settings.getLogFileName(), formater));
                m_output.Register(m_fileStream.get());
            }
        }

        LogSessionProfile& getLogSessionProfile()
        {
            return m_profile;
        }

    private:
        LogOutput m_output;
        SessionSyncStrategy m_sync;
        LogSessionProfile m_profile;
        boost::scoped_ptr<LogCoutStream> m_coutStream;
        boost::scoped_ptr<LogFileStream> m_fileStream;
    };

    // ---------------------------------------------------------------------------------------

    LogSessionProfileSetup::LogSessionProfileSetup(const LogSessionSettings &settings, 
        boost::shared_ptr<ILogRecordFormater> formater) :
        m_formater(formater),
        m_impl(new Impl(settings, m_formater))
    {}

    // ---------------------------------------------------------------------------------------

    LogSessionProfileSetup::LogSessionProfileSetup(const LogSessionSettings &settings) :
        m_formater(new DefaultFormater),
        m_impl(new Impl(settings, m_formater))
    {}

    // ---------------------------------------------------------------------------------------

    LogSessionProfileSetup::~LogSessionProfileSetup()
    {
        delete m_impl;
    }

    // ---------------------------------------------------------------------------------------

    LogSessionProfile &LogSessionProfileSetup::getLogSessionProfile()
    {
        return m_impl->getLogSessionProfile();
    }
};

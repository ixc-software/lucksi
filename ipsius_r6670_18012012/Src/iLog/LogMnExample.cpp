#include "stdafx.h"

#include "LogManager.h"
#include "LogWrapperLibrary.h"
#include "LogWrapperLoggers.h"
#include "LogManagerOutput.h"

namespace
{
    class SomeSubsystem : boost::noncopyable
    {
    public:
		SomeSubsystem(Utils::SafeRef<iLogW::ILogSessionCreator> logCreator) :
            m_session(logCreator->CreateSession("Some subsystem", true)),
            m_debugTag(m_session->RegisterRecordKind(L"Debug")),
            m_infoTag(m_session->RegisterRecordKind(L"Info"))
        {
            *m_session << m_infoTag
                << "Output text to session with tag. Some digit " << 12 << iLogW::EndRecord
                << "Output text to session without tag. Some digit " << 33 
                << m_debugTag
                << "Output text to session with tag. Some digit " << 2 << iLogW::EndRecord;  
            std::string str("Some std::string");

            *m_session << m_infoTag << str << iLogW::EndRecord;

            m_session->Add(L"debug string", m_debugTag);
            m_session->Add(L"info string", m_infoTag);
            {
                
                iLogW::LoggerTextStream logger(*m_session);
                logger.out() << "Output text without tag. Some digit " << 15;
            }
            {
                iLogW::LoggerTextStream logger(*m_session, m_debugTag);
                logger.out() << "Output text with tag. Some digit " << 25;
            }
        }
    private:
        boost::scoped_ptr<iLogW::LogSession> m_session;
        iLogW::LogRecordTag m_debugTag;
        iLogW::LogRecordTag m_infoTag;
    };

    //----------------------------------------------------------

    void Example()
    {
        iLogW::LogSettings settings;
        settings.CountSyncroToStore(100);                
        settings.TimeoutSyncroToStore(500);
        settings.TimestampInd(true);        

        settings.out().Cout().TraceInd = true;
		settings.out().File().TraceInd = true;
        settings.out().File().NameLogFile = "testLog.log";

        boost::shared_ptr<iLogW::ILogRecordFormater> formater(new iLogW::MsgBlockFormater);
        iLogW::LogManager logManager(formater, settings);
        {
            SomeSubsystem someSubsystem(&logManager);
        }
        formater.reset(new iLogW::DefaultFormater);
        logManager.Output().TraceToCoutOn(formater);
        SomeSubsystem someSubsystem(&logManager);
    }
};

namespace iLogW
{
    void LogManagerExample()
    {
        return Example();
    }
};

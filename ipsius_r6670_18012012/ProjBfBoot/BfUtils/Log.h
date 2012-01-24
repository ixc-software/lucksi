#ifndef __BFUTILSLOG__
#define __BFUTILSLOG__


#include "Utils/IBasicInterface.h"


namespace BfUtils
{

    class ILogStore : public Utils::IBasicInterface
    {
    public:
        virtual void StoreLogRecord(const std::string &s) = 0;
    };


    //-------------------------------------------------------------------------


    class Logger
    {
        ILogStore &m_store;
        std::stringstream m_stream;

    public:
        Logger(ILogStore &store) : m_store(store) {}

        ~Logger()
        {
            m_store.StoreLogRecord(m_stream.str());
        }

        std::stringstream& Out() { return m_stream; }
    };


    //-------------------------------------------------------------------------


    class LoggedClass
    {
    protected:
        ILogStore &m_logStore;
        bool m_tracingEnabled;

    public:
        LoggedClass( ILogStore &logStore, bool enableTracing )
            : m_logStore(logStore), m_tracingEnabled(enableTracing)
        {
        }


        void LogEvent(const std::string &event)
        {
            Logger logger(m_logStore);
            logger.Out() << event;
        }
    };


}  // namespace BfUtils

#endif
#include "stdafx.h"

#include "Platform/Platform.h"
#include "IntToString.h"
#include "QtMsgHandler.h"

namespace
{
    typedef Utils::QtMsgHandler::CallbackFn CallbackFn;

    QString QtMsgTypeToStr(QtMsgType type)
    {
        if (type == QtWarningMsg)   return "QtWarningMsg";
        if (type == QtDebugMsg)     return "QtDebugMsg";
        if (type == QtFatalMsg)     return "QtFatalMsg";
        if (type == QtCriticalMsg)  return "QtCriticalMsg";

        ESS_HALT( "Bad Qt msg type " + Utils::IntToString(type) );
        return "(none)";
    }

    // ------------------------------------------------------------------

    void QtHandle(QtMsgType type, const char *msg);

    class HandlersList
    {
        std::queue<CallbackFn> m_list;
        mutable Platform::Mutex m_mutex;

    public:

        HandlersList()
        {
            qInstallMsgHandler(QtHandle);
        }

        void Push(const CallbackFn &fn)
        {
            Platform::MutexLocker lock(m_mutex);
            ESS_ASSERT(fn);
            m_list.push(fn);
        }

        void Pop()
        {
            Platform::MutexLocker lock(m_mutex);
            ESS_ASSERT( !m_list.empty() );            
            m_list.pop();
        }

        CallbackFn Current() const
        {
            Platform::MutexLocker lock(m_mutex);

            CallbackFn f;

            if ( !m_list.empty() ) f = m_list.front();

            return f;
        }

    };

    bool GHaltOnMsg = false;
    HandlersList GList;

    // ------------------------------------------------------------------

    void QtHandle(QtMsgType type, const char *msg)
    {
        CallbackFn f = GList.Current();

        if (f)
        {
            f(type, QtMsgTypeToStr(type), QString(msg));
            return;
        }

        // halt if no handle 
        std::string s = QtMsgTypeToStr(type).toStdString() + ": " + std::string(msg);
        if (GHaltOnMsg) ESS_HALT(s);
        std::cerr << s << std::endl;
    }

}  // namespace 

// -------------------------------------------------------

namespace Utils
{
            
    QtMsgHandler::QtMsgHandler( CallbackFn fn )
    {
        GList.Push(fn);
    }

    QtMsgHandler::~QtMsgHandler()
    {
        GList.Pop();
    }

    void QtMsgHandler::DefaultBehaviour( bool haltOnMsg )
    {
        GHaltOnMsg = haltOnMsg;
    }

}  // namespace Utils

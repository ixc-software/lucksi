#include "stdafx.h"

#include "ProjConfigLocal.h"
#include "Platform/Platform.h"

#include "Utils/ErrorsSubsystem.h"
#include "Utils/StackString.h"
#include "Utils/ThreadNames.h"

namespace
{
    const bool CUseExceptionNameFormat = true;
    const bool CUseFunctionName = true;

    std::string FormatTypeid(const char *p)
    {
        std::string s(p);
        if ((s.size() == 0) || !(CUseExceptionNameFormat)) return s;

        try
        {
            s = Platform::FormatTypeidName(p);
            if (s.size() == 0) return p;
        }
        catch (...) 
        {
            s = p;
        }

        return s;
    }

    std::string ThreadInfo()
    {
        Platform::Thread::ThreadID id = Platform::GetCurrentThreadID();
        std::string idAsStr = Platform::Thread::ThreadIdToStr(id);
        std::string name = Platform::Thread::GetCurrentThreadName();  // can be slow, 'couse mutex usage

        if (name.empty()) return idAsStr;

        return name + " (" + idAsStr + ")";
    }

}  // namespace



// ------------------------------------------------------

namespace boost
{

    void assertion_failed(char const * expr, char const * function, char const * file, long line)
    {
        ESS::HandleIrqContext("ESS_HALT", file, function, line); 
        ESS::DoAssert( ESS::Assertion(expr, file, function, line) );
    }

} // namespace boost


// ------------------------------------------------------

namespace ESS
{
    using ProjConfig::CfgESS;

    // template instance
    template bool DoException<TutException>(const string& textMsg, const char *pFile, const char *pFunc, int line);

    // -------------------------------------------------

    void BaseException::SetMsg(string &msg, bool addStackTrace) const
    {
        std::ostringstream ss;

        ss << "Exception ";

        if (!m_textMsg.empty())
        {
            ss << '"' << m_textMsg << '"';
        }

        ss << "; ";

        ss << FormatTypeid( typeid(*this).name() );
        ss << " in thread " << ThreadInfo();
        
        string location = getLocation();
        if (!location.empty()) 
        {
            ss << " @ " << location;
        }

        if ((!m_trace.empty()) && (addStackTrace)) 
        {
            ss << endl << "* Stack trace *" << endl << m_trace;
        }

        msg = ss.str();
    }

    void BaseException::NoStackTraceInMsg()
    {
        m_msgAddStackTrace = false;
        if (m_msg.empty()) return;
        SetMsg(m_msg, m_msgAddStackTrace);
    }

    string BaseException::getLocation() const
    {
        std::ostringstream ss;

        if (m_pFile != 0) 
        {
            ss << m_pFile << ":" << m_line;

            if (CUseFunctionName && (m_pFunc != 0))
            {
                std::string f(m_pFunc);
                if (!f.empty())
                {
                    ss << " in " << f;
                }
            }
        }

        return ss.str();
    }

    string BaseException::getTextAndPlace() const
    {
        string res;

        res = "\"" + getTextMessage() + "\"";

        string location = getLocation();
        if (location.size() > 0)
        {
            res += " @ " + location;
        }

        return res;
    }

    const char* BaseException::what() const throw()
    {
        if (m_msg.empty()) SetMsg(m_msg, m_msgAddStackTrace);

        return m_msg.c_str();
    }

    BaseException::BaseException( const string& textMsg, 
        const char *pFile, const char *pFunc, int line, 
        bool forcedStackTraceCapture) : 
        m_pFile(pFile), m_pFunc(pFunc), m_line(line), 
        m_textMsg(textMsg), 
        m_msgAddStackTrace(true)
    {
        if (forcedStackTraceCapture || CfgESS::CStackTraceInExceptions)
        {
            if (CfgESS::CUseStackTrace) Platform::GetStackTrace(m_trace);
        }
    }
    // -------------------------------------------------

    void _ExceptionBreak(const string& textMsg, const char *pFile, int line) {}

    // -------------------------------------------------

    class HooksList
    {    
        typedef std::list<ExceptionHook*> List;
        List m_list;  // required mutex access for thread-safety

        List::iterator Find(ExceptionHook* pHook)
        {
            return std::find(m_list.begin(), m_list.end(), pHook);
        }

        ESS_TYPEDEF_FULL(ExceptionInHook, Assertion);

        void Abort(const std::exception *pE)
        {
            using std::string;

            string msg = "Exception in exception hook, aborted";
            if (pE) msg += string("\n") + string(pE->what());

            ExceptionInHook e(msg, __FILE__, BOOST_CURRENT_FUNCTION, __LINE__);
            DoAssert(e, false);
        }

    public:

        void Add(ExceptionHook* pHook)
        {
            // dublicate find
            List::iterator p = Find(pHook);
            ESS_ASSERT(p == m_list.end());

            // add
            m_list.push_front(pHook);
        }

        void Remove(ExceptionHook* pHook)
        {
            // find
            List::iterator p = Find(pHook);
            ESS_ASSERT(p != m_list.end());

            // remove
            m_list.erase(p);
        }

        void Process(const BaseException *pE)
        {
            try
            {
                List::iterator i = m_list.begin();

                while(i != m_list.end())
                {
                    (*i)->Hook(pE);
                    ++i;
                }
            }
            catch(/*const*/ BreakHookChain&)
            {
                // прерываем цепочку обработчиков
                return;  
            }
            catch(/*const*/ HookRethrow&)
            {
                // замещаем обрабатываемое исключение этим
                throw;
            }
            catch(/*const*/ std::exception &e)
            {
                Abort(&e);
            }
            catch(...)
            {
                Abort(0);
            }
        }

    };

    namespace
    {
        HooksList *pHooksList = 0;
    }

    // -------------------------------------------------

    void RegisterHook(ExceptionHook *pHook)
    {
        if (!pHooksList) pHooksList = new HooksList();
        ESS_ASSERT(pHook);
        pHooksList->Add(pHook);
    }

    void UnregisterHook(ExceptionHook *pHook)
    {
        ESS_ASSERT(pHooksList);
        ESS_ASSERT(pHook);
        pHooksList->Remove(pHook);
    }

    void ProcessHook(const BaseException *pE)
    {
        ESS_ASSERT(pE);

        if (pHooksList) pHooksList->Process(pE);        
    }

    // -------------------------------------------------

    /*
    bool DoTUT(const TutException &e)
    {
    throw e;
    return true;
    }
    */

    bool DoAssert(const Assertion &e, bool processHook)
    {
        if (processHook) ProcessHook(&e);

        Platform::ExitWithMessage(e.what());

        // never goes here
        return true;
    }

    bool UnexpectedException(const std::exception &e, const char *pFile, const char *pFunc, int line)
    {
        std::string msg = " Unexpected exception ";
        msg += typeid(e).name();
        msg += " [";
        msg += e.what();
        msg += "]\n";

        DoAssert( Assertion(msg, pFile, pFunc, line) );
        return true;
    }

    /*
    bool DoException(const Exception &e)
    {
    throw e;
    return true;
    } */


    bool HandleIrqContext(const char *pMsg, const char *pFile, const char *pFunc, int line)
    {
        if (!Platform::InsideIrqContext()) return false;

        // Make info string
        // Note: pFunc don't used (Blackfin don't have this macro)
        Utils::StackString s;
        s += "ASSERT ";
        if (pMsg != 0) s += pMsg;
        s += " @";
        s += pFile;
        s += ":";
        s.AddInt(line);
        
        Platform::ExitWithMessage(s.c_str());

        // never goes here
        return true;
    }

}  // namespace


namespace
{
    ESS_TYPEDEF(TestException);


    void Test(bool b)
    {
        TUT_ASSERT(b);
        ESS_ASSERT(b);
        ESS_THROW(TestException);
    }
}

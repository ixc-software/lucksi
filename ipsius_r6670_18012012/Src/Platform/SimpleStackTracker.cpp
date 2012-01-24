#include "StdAfx.h"
#include "SimpleStackTracker.h"
#include "StackWalker.h"


struct CallstackEntryEx : public StackWalker::CallstackEntry {};

// typedef StackWalker::CallstackEntry CallstackEntryEx;

// --------------------------------------------------------

StackTraceItem::StackTraceItem(CallstackEntryEx &entry) 
    : m_name(entry.name), m_line(entry.lineNumber), 
    m_fileName(entry.lineFileName), m_module(entry.moduleName)
{
    if (m_name.length() > 0) m_name += "()";
}

// --------------------------------------------------------

class StackWalkerEx : public StackWalker
{
    typedef std::vector<std::string> List;

    const StackItemFormater *m_pFormater; 
    bool m_useFilter;
    List m_list;

    // override
    void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
    {
        if (eType == lastEntry) return;
        if (entry.offset == 0) return;

        // cast связан с невозможностью "показать" StackWalker::CallstackEntry в SimpleStackTracker.h
        StackTraceItem item(static_cast<CallstackEntryEx&>(entry));  

        m_list.push_back( m_pFormater->FormatItem(item) );
    }

    std::string GetText(bool useFilter) 
    { 
        bool addItems = (useFilter) ? false : true;
        std::string result;

        for(List::iterator i = m_list.begin(); i != m_list.end(); ++i)
        {
            std::string s = *i;

            if ((useFilter) && !addItems)
            {
                size_t pos = s.find("ESS::BaseException::BaseException");
                if (pos != std::string::npos)
                {
                    addItems = true;
                    continue;
                }
            }

            if (addItems) result += s;
        }

        return result; 
    }

public:

    StackWalkerEx() : StackWalker(), m_pFormater(0), m_useFilter(false)
    { 
    }

    std::string getText(const StackItemFormater *pFormater, bool useFilter) 
    { 
        // init
        m_list.clear();
        m_pFormater = pFormater;
        m_useFilter = useFilter;

        // capture stack
        ShowCallstack();

        // make result
        std::string result = GetText(m_useFilter);

        // если фильтр не сработал, отдать результаты без фильтра
        if ((result.empty()) && m_useFilter) result = GetText(false);

        return result; 
    }

};

// --------------------------------------------------------

namespace
{
    boost::shared_ptr<StackWalkerEx> GTracer;
}

std::string SimpleStackTracker::GetCurrentTrace(bool useFilter)
{
    StackItemFormaterShort formater;
    return GetCurrentTrace(formater, useFilter);
}

std::string SimpleStackTracker::GetCurrentTrace(const StackItemFormater &formater, bool useFilter)
{
    #ifdef DISABLE_WIN32_STACKTRACE

        return "Stack trace disabled at build";

    #else

        try
        {
            if (GTracer.get() == 0) GTracer.reset( new StackWalkerEx());
            return GTracer->getText(&formater, useFilter);
        }
        catch (const std::exception &e)
        {
            return "StackWalkerEx Exception: " + std::string(e.what());
        }
        catch (...)
        {
            return "StackWalkerEx: Unexpected exception occured";
        }

    #endif
}


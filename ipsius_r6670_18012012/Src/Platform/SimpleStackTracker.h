#ifndef __SIMPLESTACKTRACKER__
#define __SIMPLESTACKTRACKER__

#include "stdafx.h"

struct CallstackEntryEx;

class StackTraceItem
{
    std::string m_name;
    int m_line;
    std::string m_fileName;
    std::string m_module;

public:
    StackTraceItem(CallstackEntryEx &entry);

    const std::string& getName() const { return m_name; }
    int getLine() const { return m_line; }
    const std::string& getFileName() const { return m_fileName; }
    const std::string& getModule() const { return m_module; }

    std::string GetNameWithFix() const
    {
        return (m_name.empty()) ? "<undefined>" : m_name;
    }
};

// ----------------------------------------------------------

class StackItemFormater
{
    virtual void DoFormat(std::stringstream &ss, const StackTraceItem &item) const = 0;

public:
    virtual ~StackItemFormater()
    {
    }
    
    std::string FormatItem(const StackTraceItem &item) const
    {
        std::stringstream ss;
        DoFormat(ss, item);
        ss << "\n";
        return ss.str();
    }
};

class StackItemFormaterShort : public StackItemFormater
{
    void DoFormat(std::stringstream &ss, const StackTraceItem &item) const  // override
    {
        ss << item.GetNameWithFix();
    }

};

// ----------------------------------------------------------

class SimpleStackTracker
{
public:
	static std::string GetCurrentTrace(bool useFilter = true); 
    static std::string GetCurrentTrace(const StackItemFormater &formater, bool useFilter = true); 
};

#endif


#include "stdafx.h"

#include "TestUtils.h"

#include "iCore/SyncTimer.h"
#include "iCore/MsgThread.h"


namespace TestUtils
{
    TestLog::TestLog(string testName, string data) 
        : m_testName(testName), m_data(data)
    {
        m_data += "\n";
    }

    // ----------------------------------------

    void TestLog::setTestName(string name)
    {
        m_testName = name;
    }

    // ----------------------------------------

    std::string TestLog::getTestName()
    {
        return m_testName;
    }

    // ----------------------------------------

    void TestLog::Log(string data)
    {
        m_data += data;
        m_data +="\n";
    }

    // ----------------------------------------

    void TestLog::Log(int data)
    {
        std::stringstream s;
        s << data << "/n";

        m_data += string(s.str());
    }

    // ----------------------------------------

    void TestLog::Log(string data1, int data2)
    {
        std::stringstream s;
        s << data1 << data2 << endl;

        m_data += string(s.str());
    }

    // ----------------------------------------

    std::string TestLog::AsString()
    {
        std::stringstream s;
        s << endl << "Test log of test '" << m_testName << "' : "
            << endl << m_data;

        return string(s.str());
    }

    // ----------------------------------------

    TestProcess::TestProcess(dword testDurationMS) 
        : m_testDurationMS(testDurationMS)
    {}

    // ----------------------------------------

    void TestProcess::Run()
    {
        iCore::SyncTimer t;

        while (true)
        {
            if (m_testDurationMS)
            {
                if (t.Get() >= m_testDurationMS) break;
            }

            Platform::Thread::Sleep(20);
            if(IsProcessed()) return;
        }
        
        if (TimeoutTestFail()) TUT_ASSERT(0 && "Wait fail!");
    }

    // ----------------------------------------

    /*
    namespace 
    {
        using std::string;

        // for string parser
        void AddData(const string& source, 
                     Utils::ManagedList<string>& dest,
                     bool removeEmpty)
        {
            if ((source.empty()) && (removeEmpty)) return;

            dest.Add(new string(source));
        }
    }

    // ----------------------------------------

    
    //true - string parsed, false - str or separator are empty
    void StringParser(const string& inStr, 
                      const string& inSeparator, 
                      Utils::ManagedList<string>& outData,
                      bool removeEmpty)
    {
        outData.Clear();

        //if no separator, return full input string
        if (inSeparator.empty()) 
        {
            AddData(inStr, outData, removeEmpty);
            return;
        }

        // if separtor exists, parse string 
        size_t prev = 0;
        while (true)
        {
            size_t next = inStr.find_first_of(inSeparator, prev);

            // copy symbols after last separator || 
            // copy all string if separator's not found
            if (next == string::npos)  
            {
                AddData(inStr.substr(prev), outData, removeEmpty);
                break;
            }

            AddData(inStr.substr(prev, (next - prev)), outData, removeEmpty);

            prev = next + inSeparator.size();
        }
    }
    */
    // ----------------------------------------------
    /*
    int StrToInt(const string& s)
    {
        std::istringstream iss(s);
        int val;
        iss >> val;
        return val;
    }
    */

} //namespace TestUtils

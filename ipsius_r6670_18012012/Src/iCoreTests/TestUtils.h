#ifndef __TESTUTILS__
#define __TESTUTILS__

#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/ManagedList.h"


namespace TestUtils
{
    using std::string;
    using std::endl;
    using Platform::dword;

    class TestLog
    {
        string m_testName;
        string m_data;

    public:
        TestLog(string testName = "", string data = ""); 

        void setTestName(string name);
        string getTestName();

        void Log(string data);
        void Log(int data);
        void Log(string data1, int data2);
        string AsString();

        template<class T>
        void WriteTo(T& io)
        {
            io << AsString();
        }
   };

   // ----------------------------------------

   class TestProcess
   {
        dword m_testDurationMS;

    protected:
        virtual bool IsProcessed() = 0;
        virtual bool TimeoutTestFail() { return true; }  // throw exception if return true

    public:
        TestProcess(dword testDurationMS);
        virtual ~TestProcess() {};

        void Run();
    };


    /*
    void StringParser(const string& inStr, 
                      const string& inSeparator, 
                      Utils::ManagedList<string>& outData,
                      bool removeEmpty = false);  

    */

} // namespace  TestUtils

#endif


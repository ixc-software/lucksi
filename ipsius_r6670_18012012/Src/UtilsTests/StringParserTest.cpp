
#include "stdafx.h"

#include "StringParserTest.h"

#include "Utils/StringParser.h"
#include "Platform/Platform.h"

namespace 
{
    void Test(const std::string &data, 
              const std::string &separator,
              bool removeEmpty,
              std::vector<std::string> &checkData)
    {
        Utils::ManagedList<std::string> parsed;
        Utils::StringParser(data, separator, parsed, removeEmpty);

        TUT_ASSERT(parsed.Size() == checkData.size());
        if (parsed.Size() == 0) return;

        for (size_t i = 0; i < parsed.Size(); ++ i)
        {
            TUT_ASSERT(*parsed[i] == checkData.at(i));
        }
    }
    
} // namespace 

// -----------------------------------------------------

namespace UtilsTests
{
    void StringParserTest()
    {
        {
            std::vector<std::string> checkData;
            checkData.push_back("123");
            checkData.push_back("456");
            checkData.push_back("78");
            checkData.push_back("9");
            checkData.push_back("0");
            Test("123_456_78_9_0", "_", false, checkData);
        }
        
        {
            std::vector<std::string> checkData;
            checkData.push_back("a");
            checkData.push_back("b");
            checkData.push_back("c");
            checkData.push_back("");
            Test("a123b123c123", "123", false, checkData);
        }
        
        {
            std::vector<std::string> checkData;
            Test("+++", "+", true, checkData);
        }
        
        {
            std::vector<std::string> checkData;
            checkData.push_back("  23 ");
            Test("  23 ", ", ", false, checkData);
        }
        
        {
            std::vector<std::string> checkData;
            checkData.push_back("1..15 ");
            checkData.push_back("23 ");
            checkData.push_back("17..31");
            Test("1..15 , 23 , 17..31", ", ", false, checkData); 
        }
    }

} // namespace UtilsTests 

#ifndef __BFCONFIGCOMMON__
#define __BFCONFIGCOMMON__


#include "Utils/ErrorsSubsystem.h"


namespace BfConfig
{

    // Класс со статическими функциями, общими для BfConfigComposer и BfConfigParser
    class BfConfigCommon
    {
    public:
        typedef std::pair<std::string, std::string> KeyValuePair;
        typedef std::vector<std::string> StringList;


        // Trims string str
        static std::string Trim(const std::string &str, const std::string charList = " \x09")
        {
            std::string temp = str;
            std::string::size_type pos = temp.find_last_not_of(charList);

            if ( pos != std::string::npos )
            {
                temp.erase(pos + 1);

                pos = temp.find_first_not_of(charList);

                if (pos != std::string::npos) temp.erase(0, pos);
            }
            else
            {
                temp.erase(temp.begin(), temp.end());
            }

            return temp;
        }


        //-------------------------------------------------------------------------


        static std::string Lowercase(const std::string &in)
        {
            std::string out = in;
            transform(in.begin(), in.end(), out.begin(), tolower);

            return out;
        }


        //-------------------------------------------------------------------------


        // Checks Key name
        static bool IsKeyValid(const std::string &key)
        {
            if (key.empty()) return false;

            for (int i = 0; i < key.length(); ++i)
            {
                bool isLower     = (key[i] >= 'a') && (key[i] <= 'z');
                bool isUpper     = (key[i] >= 'A') && (key[i] <= 'Z');
                bool isDigit     = (key[i] >= '0') && (key[i] <= '9');
                bool isSeparator = key[i] == '_';

                if ((i == 0) && (isDigit)) return false;
                if ( !(isLower || isUpper || isDigit || isSeparator) ) return false;
            }

            return true;
        }


        //-------------------------------------------------------------------------


        static int FindRecord(const std::vector<KeyValuePair> &config, const std::string &key)
        {
            for(int i = 0; i < config.size(); ++i)
            {
                if (config.at(i).first == key) return(i);
            }

            return -1;
        }


        //-------------------------------------------------------------------------


        static void Test()
        {
            // Test Trim()
            TUT_ASSERT( "_string_" == Trim("\x09 _string_ \x09") );

            // Test TryLowercase()
            TUT_ASSERT( "lowercase" == Lowercase("LOWERCASE") );

            // Test IsKeyValid()
            TUT_ASSERT( IsKeyValid("1_param") == false );
            TUT_ASSERT( IsKeyValid(" param")  == false );
            TUT_ASSERT( IsKeyValid("param-")  == false );
            TUT_ASSERT( IsKeyValid("Param")   != false );
            TUT_ASSERT( IsKeyValid("_1param") != false );

            // Test FindRecord()
            std::vector<KeyValuePair> testVector;
            testVector.push_back( KeyValuePair("param1", "value1") );
            testVector.push_back( KeyValuePair("param2", "value2") );
            testVector.push_back( KeyValuePair("param3", "value3") );

            TUT_ASSERT( FindRecord(testVector, "param1") == 0 );
            TUT_ASSERT( FindRecord(testVector, "param2") == 1 );
            TUT_ASSERT( FindRecord(testVector, "param3") == 2 );
        }
    };

}


#endif

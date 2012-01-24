#include "stdafx.h"
#include "BfConfigParser.h"


using namespace std;
using std::string;
using std::vector;


namespace BfConfig
{

    //-------------------------------------------------------------------------
    // BfConfigParser implementation
    //-------------------------------------------------------------------------


    BfConfigParser::BfConfigParser(const BfConfigParser::StringList &config, bool caseSensitive) :
        m_caseSensitive(caseSensitive)
    {
        typedef StringList::const_iterator ConstIter;

        // Get each record from the config string list and store Key and its Value
         
        for (ConstIter it = config.begin(); it != config.end(); ++it)
        {
            // Get another record
            string record = *it;

            // Skip if empty
            if ( record.empty() ) continue;

            // Split it into the Key name and its Value

            // Get and check Value and Description offsets
            string::size_type valueOffset = record.find('=');
            if (string::npos == valueOffset) throw ESS_THROW(ValueSeparatorNotFound);

            // Extract and check Key name
            string key = BfConfigCommon::Trim(record.substr(0, valueOffset)); 
            key = TryLowercase(key);
            if (!BfConfigCommon::IsKeyValid(key)) throw ESS_THROW(IllegalKeyName);

            // Extract and check Value
            string value;
            value = BfConfigCommon::Trim( record.substr(valueOffset + 1) );

            // Duplicates check
            if (FindRecord(m_config, key) >= 0) throw ESS_THROW(DuplicateKeyFound);

            // Key and it's value are valid. Store parameter
            m_config.push_back( KeyValuePair(key, value) );
        }
    }


    //-------------------------------------------------------------------------


    BfConfigParser::~BfConfigParser()
    {
    }


    //-------------------------------------------------------------------------


    string BfConfigParser::TryLowercase(const string &in) const
    {
        if (m_caseSensitive) return in;

        return BfConfigCommon::Lowercase(in);
    }


    //-------------------------------------------------------------------------


    int BfConfigParser::GetVariablesCount() const
    {
        return m_config.size();
    }


    //-------------------------------------------------------------------------


    void BfConfigParser::GetVariable(int index, string &name, string &value) const
    {
        ESS_ASSERT( (index >= 0) || (index < GetVariablesCount()) );

        name = m_config.at(index).first;
        value = m_config.at(index).second;
    }

} // namespace BfConfig


//-----------------------------------------------------------------------------


// BfConfigParser test
namespace
{
    using namespace BfConfig;
    typedef BfConfig::BfConfigCommon::StringList StringList;

    void AssertWrongParams(const StringList &sl)
    {
        bool isOk = false;

        try
        {
            BfConfigParser testParser(sl);
        }
        catch (BfConfigParser::ParsingError &e)
        {
            isOk = true;
        }

        TUT_ASSERT(isOk);
    }


    //-------------------------------------------------------------------------


    void TestWrongParameters()
    {
        // Some string in a string list isn't empty but doesn't contain '='
        {
            StringList sl;
            sl.push_back("param");
            AssertWrongParams(sl);
        }

        // First symbol in a parameter name is a digit
        {
            StringList sl;
            sl.push_back("1param=something");
            AssertWrongParams(sl);
        }

        // Some symbol in a parameter name is not a digit, Latin symbol, separator ('_')
        {
            StringList sl;
            sl.push_back("wrong-param=something");
            AssertWrongParams(sl);
        }

        // Duplicate parameter
        {
            const string duplicate = "Parameter_1=something";
            StringList sl;
            sl.push_back(duplicate);
            sl.push_back("Parameter_2=something");
            sl.push_back(duplicate);
            AssertWrongParams(sl);
        }
    }


    //-------------------------------------------------------------------------


    void TestTrimmer()
    {
        const string trimmedParam = "parameter_with_tab_and_space_on_both_sides";

        BfConfig::BfConfigCommon::StringList config;
        config.push_back("\x09 " + trimmedParam + "\x09 =correct");

        BfConfigParser testParser(config, false);

        string name, value;
        testParser.GetVariable(0, name, value);
        TUT_ASSERT(trimmedParam == name);
        TUT_ASSERT("correct" == value);
    }


    //-------------------------------------------------------------------------


    void TestCaseSensitivity()
    {
        BfConfig::BfConfigCommon::StringList testConfig;

        testConfig.push_back("Parameter=correct");

        BfConfigParser caseSensitive(testConfig, true);
        BfConfigParser caseInsensitive(testConfig, false);

        string name, value;

        // Test case sensitive config
        caseSensitive.GetVariable(0, name, value);
        TUT_ASSERT("Parameter" == name);

        // Test case insensitive config
        caseInsensitive.GetVariable(0, name, value);
        TUT_ASSERT("parameter" == name);
    }
}


//-----------------------------------------------------------------------------


namespace BfConfig
{
    void BfConfigParser::Test()
    {
        // Test parser with wrong parameters
        TestWrongParameters();

        // All parameters are correct and some of them have to be trimmed
        TestTrimmer();

        // Test case sensitive and case insensitive configs
        TestCaseSensitivity();
    }

} // namespace BfConfig

#include "stdafx.h"
#include "BfConfigComposer.h"


using std::string;
using std::vector;


namespace BfConfig
{

    BfConfigComposer::BfConfigComposer(bool caseSensitive) :
        m_caseSensitive(caseSensitive)
    {

    }


    //-------------------------------------------------------------------------


    BfConfigComposer::~BfConfigComposer()
    {

    }


    //-------------------------------------------------------------------------


    string BfConfigComposer::TryLowercase(const string &in) const
    {
        if (m_caseSensitive) return in;

        return BfConfigCommon::Lowercase(in);
    }


    //-------------------------------------------------------------------------


    void BfConfigComposer::AddVariable(const string& name, const string& value)
    {
        string keyName = TryLowercase(BfConfigCommon::Trim(name));

        if ( BfConfigCommon::IsKeyValid(keyName) != true ) ESS_THROW(IllegalKeyName);

        KeyValuePair pair;

        pair.first  = keyName;
        pair.second = value;

        typedef vector<KeyValuePair>::iterator Iterator;
        for (Iterator it = m_config.begin(); it != m_config.end(); ++ it)
        {
            KeyValuePair pair;

            if (name == it->first) ESS_THROW(DuplicateKeyFound);
        }

        m_config.push_back(pair);
    }


    //-------------------------------------------------------------------------


    void BfConfigComposer::GetConfig(StringList &config)
    {
        config.clear();

        typedef vector<KeyValuePair>::iterator Iterator;
        for (Iterator it = m_config.begin(); it != m_config.end(); ++it)
        {
            config.push_back(it->first + "=" + it->second);
        }
    }


    //-------------------------------------------------------------------------


    void BfConfigComposer::Test()
    {
        // Compose Config test
        {
            BfConfigComposer configComposer(true);

            configComposer.AddVariable("Test_param1", "param1");
            configComposer.AddVariable("\x9 Test_param2 \x9", "param2");
            configComposer.AddVariable("_", "param3");

            StringList config;

            configComposer.GetConfig(config);

            TUT_ASSERT( "Test_param1=param1" == config.at(0) );
            TUT_ASSERT( "Test_param2=param2" == config.at(1) );
            TUT_ASSERT( "_=param3" == config.at(2) );
        }


        // Duplicates and case insensitivity test
        {
            BfConfigComposer configComposer(false);
            bool isOk = false;

            try
            {
                configComposer.AddVariable("Case_insensitive", "param1"); // Will be lowercased into "case_insensitive"
                configComposer.AddVariable("case_insensitive", "param2"); // Try to add duplicate
            }
            catch (DuplicateKeyFound &e)
            {
                isOk = true;
            }

            TUT_ASSERT( isOk );
        }
    }

} // namespace BfConfig

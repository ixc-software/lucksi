#include "stdafx.h"
#include "BfServerConfig.h"

#include "BfUtils/StringConvertions.h"
#include "BfConfig/BfConfigParser.h"
#include "BfConfig/BfConfigComposer.h"



namespace BfServer
{
    using namespace BfUtils;
    using namespace BfConfig;
    using std::string;


    //-------------------------------------------------------------------------
    // ConfigVaribaleBase implementation
    //-------------------------------------------------------------------------


    ConfigVariableBase::ConfigVariableBase(const string &name, const string &desc, const string &type) :
        m_name(name),
        m_description(desc),
        m_type(type)
    {
        ESS_ASSERT( false == name.empty() );
        ESS_ASSERT( false == desc.empty() );
    }


    //-------------------------------------------------------------------------


    ConfigVariableBase::~ConfigVariableBase()
    {

    }


    //-------------------------------------------------------------------------


    const string& ConfigVariableBase::getName() const
    {
        return m_name;
    }


    //-------------------------------------------------------------------------


    const string& ConfigVariableBase::getDescription() const
    {
        return m_description;
    }


    //-------------------------------------------------------------------------


    const string& ConfigVariableBase::getType()
    {
        return m_type;
    }


    //-------------------------------------------------------------------------


    string ConfigVariableBase::getValueAsString() const
    {
        return ToString();
    }


    //-------------------------------------------------------------------------


    bool ConfigVariableBase::setValueFromString(const string &s)
    {
        return FromString(s);
    }


    //-------------------------------------------------------------------------
    // ConfigVariableInt implementation
    //-------------------------------------------------------------------------


    ConfigVariableInt::ConfigVariableInt(const string &name, const string &description, int value, int minValue, int maxValue) :
        ConfigVariableBase(name, description, "Integer"),
        m_value(value),
        m_maxValue(maxValue),
        m_minValue(minValue),
        m_hasValueRestrictions(true)
    {
        ESS_ASSERT( minValue < maxValue );
        ESS_ASSERT( (value >= minValue) && (value <= maxValue) );
    }


    //-------------------------------------------------------------------------


    ConfigVariableInt::ConfigVariableInt(const string &name, const string &description, int value) :
        ConfigVariableBase(name, description, "Integer"),
        m_value(value),
        m_minValue(0),
        m_maxValue(0),
        m_hasValueRestrictions(false)
    {
    }


    //-------------------------------------------------------------------------


    ConfigVariableInt::~ConfigVariableInt()
    {
    }


    //-------------------------------------------------------------------------


    int ConfigVariableInt::getValue() const
    {
        return m_value;
    }


    //-------------------------------------------------------------------------


    void ConfigVariableInt::setValue(int value)
    {
        ESS_ASSERT( (!m_hasValueRestrictions) ||
                    ( (value >= m_minValue) && (value <= m_maxValue) ) );

        m_value = value;
    }


    //-------------------------------------------------------------------------


    bool ConfigVariableInt::TrySetValue(int value)
    {
        if ( (value < m_minValue) || (value > m_maxValue) ) return false;

        m_value = value;

        return true;
    }


    //-------------------------------------------------------------------------
    // ConfigVariableBase implementation


    string ConfigVariableInt::ToString() const
    {
        return BfUtils::ToString<int>(m_value);
    }


    //-------------------------------------------------------------------------


    bool ConfigVariableInt::FromString(const string &valueString)
    {
        return TrySetValue( BfUtils::FromString<int>(valueString) );
    }


    //-------------------------------------------------------------------------
    // ConfigVariableString
    //-------------------------------------------------------------------------


    ConfigVariableString::ConfigVariableString(const string &name, const string &description, const string &value, bool mayBeEmpty) :
        ConfigVariableBase(name, description, "string"),
        m_value(value),
        m_mayBeEmpty(mayBeEmpty)
    {

    }

        
    //-------------------------------------------------------------------------


    ConfigVariableString::~ConfigVariableString()
    {
    }


    //-------------------------------------------------------------------------


    const string& ConfigVariableString::getValue() const
    {
        return m_value;
    }


    //-------------------------------------------------------------------------


    void ConfigVariableString::setValue(const string &value)
    {
        ESS_ASSERT( m_mayBeEmpty || !value.empty() );

        m_value = value;
    }


    //-------------------------------------------------------------------------


    bool ConfigVariableString::TrySetValue(const string &value)
    {
        if (!m_mayBeEmpty && value.empty()) return false;

        m_value = value;

        return true;
    }


    //-------------------------------------------------------------------------
    // ConfigVariableBasic implementation

    string ConfigVariableString::ToString() const
    {
        return m_value;
    }


    //-------------------------------------------------------------------------


    bool ConfigVariableString::FromString(const string &valueString)
    {
        return TrySetValue(valueString);
    }


    //-------------------------------------------------------------------------
    // BfServerConfig implementation
    //-------------------------------------------------------------------------


    const string CWaitConnectionTimeoutDesc       = "todo: wait_connection_timeout description";
    const string CSocketListenerPollingPeriodDesc = "todo: socket_listener_polling_period description";
    const string CTransportPollingPeriodDesc      = "todo: transport_polling_period description";
    const string CProtocolPollingPeriodDesc       = "todo: protocol_polling_period description";
    const string CSessionClearingPeriodDesc       = "todo: session_clearing_period description";


    BfServerConfig::BfServerConfig(const BfConfig::BfConfigCommon::StringList &config) :
        WaitConnectionTimeout      ("wait_connection_timeout",        CWaitConnectionTimeoutDesc      , 5000, 500, 10000 ),
        SocketListenerPollingPeriod("socket_listener_polling_period", CSocketListenerPollingPeriodDesc,  100,  25,   500 ),
        TransportPollingPeriod     ("transport_polling_period",       CTransportPollingPeriodDesc     ,   50,  25,   500 ),
        ProtocolPollingPeriod      ("protocol_polling_period",        CProtocolPollingPeriodDesc      ,   50,  25,   500 ),
        SessionClearingPeriod      ("session_clearing_period",        CSessionClearingPeriodDesc      , 1000, 500,  5000 )
    {
//        AddVariable(WaitConnectionTimeout);
//        AddVariable(SocketListenerPollingPeriod);
//        AddVariable(TransportPollingPeriod);
//        AddVariable(ProtocolPollingPeriod);
//        AddVariable(SessionClearingPeriod);
    }


    //-------------------------------------------------------------------------


    BfServerConfig::~BfServerConfig()
    {
    }


    //-------------------------------------------------------------------------


    void BfServerConfig::AddVariable(ConfigVariableBase &variable)
    {
        m_variables.push_back(&variable);
    }


    //-------------------------------------------------------------------------


    void BfServerConfig::GetConfig(BfConfig::BfConfigCommon::StringList &config, bool caseSensitive)
    {
        BfConfigComposer configComposer(caseSensitive);

        std::vector<ConfigVariableBase*>::iterator it = m_variables.begin();
        for(; it != m_variables.end(); ++it)
        {
            configComposer.AddVariable((*it)->getName(), (*it)->getValueAsString());
        }

        configComposer.GetConfig(config);
    }


    //-------------------------------------------------------------------------


    // todo: Убрать
    void BfServerConfig::LogVariables()
    {
        std::vector<ConfigVariableBase*>::iterator it = m_variables.begin();

        for (; it != m_variables.end(); ++it)
        {
            std::cout << "Key name:           " << (*it)->getName() << std::endl;
            std::cout << "Key value:          " << (*it)->getValueAsString() << std::endl;
            std::cout << "Key description:    " << (*it)->getDescription() << std::endl;
            std::cout << std::endl;
        }
    }

}

#ifndef __BFSERVERCONFIG__
#define __BFSERVERCONFIG__


#include "Utils/ErrorsSubsystem.h"
#include "BfUtils/Log.h"
#include "BfConfig/BfConfigParser.h"
#include "BfConfig/BfConfigComposer.h"


namespace BfServer
{
    using boost::scoped_ptr;


    // Базовый класс для переменных окружения
    class ConfigVariableBase
    {
        std::string m_name;
        std::string m_description;
        std::string m_type;

        virtual std::string ToString() const = 0;
        virtual bool FromString(const std::string &s) = 0;


    public:
        ConfigVariableBase(const std::string &name, const std::string &desc, const std::string &type);
        ~ConfigVariableBase();

        const std::string& getName() const;
        const std::string& getDescription() const;

        const std::string& getType();

        std::string getValueAsString() const;
        bool setValueFromString(const std::string &valueString);
    };


    //-------------------------------------------------------------------------


    class ConfigVariableInt : public ConfigVariableBase
    {
        int     m_value;

        int     m_minValue;
        int     m_maxValue;

        bool    m_hasValueRestrictions;


    public:
        ConfigVariableInt(const std::string &name, const std::string &description, int value, int minValue, int maxValue);
        ConfigVariableInt(const std::string &name, const std::string &description, int value);
        ~ConfigVariableInt();

        int    getValue() const;
        void   setValue(int value);
        bool   TrySetValue(int value);


    // ConfigVariableBase implementation
    private:
        std::string ToString() const;
        bool FromString(const std::string &valueString);
    };


    //-------------------------------------------------------------------------


    class ConfigVariableString : public ConfigVariableBase
    {
        std::string  m_value;
        bool    m_mayBeEmpty;


    public:
        ConfigVariableString(const std::string &name, const std::string &description, const std::string &value, bool mayBeEmpty);
        ~ConfigVariableString();

        const std::string& getValue() const;
        void          setValue(const std::string &value);
        bool          TrySetValue(const std::string &value);


    // ConfigVariableBase implementation
    private:
        std::string ToString() const;
        bool FromString(const std::string &valueString);
    };


    //-------------------------------------------------------------------------


    // Класс для обработки, получения, изменения и сохранения конфигурации Сервера
    class BfServerConfig
    {
        std::vector<ConfigVariableBase*> m_variables;

        void AddVariable(ConfigVariableBase &variable);


    public:
        // Predefined configuration variables
        const ConfigVariableInt WaitConnectionTimeout;
        const ConfigVariableInt SocketListenerPollingPeriod;
        const ConfigVariableInt TransportPollingPeriod;
        const ConfigVariableInt ProtocolPollingPeriod;
        const ConfigVariableInt SessionClearingPeriod;

        BfServerConfig(const BfConfig::BfConfigCommon::StringList &config);
        ~BfServerConfig();

        void GetConfig(BfConfig::BfConfigCommon::StringList &config, bool caseSensitive);

        void LogVariables();
    };

}


#endif

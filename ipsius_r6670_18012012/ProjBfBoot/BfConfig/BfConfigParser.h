#ifndef __BFCONFIGPARSER__
#define __BFCONFIGPARSER__


#include "Utils/ErrorsSubsystem.h"
#include "Platform/Platform.h"

#include "BfConfigCommon.h"


namespace BfConfig
{
    using namespace Platform;

    // ����� ������ ��� ����-��������. ����� ��������� ���� ���������� ��
    // ������� ��������� ��� "����=��������"
    class BfConfigParser : private BfConfigCommon
    {
        std::vector<KeyValuePair> m_config;
        bool m_caseSensitive;

        std::string TryLowercase(const std::string &in) const;


    public:
        // ������ ������ ������� �������
        ESS_TYPEDEF(ParsingError);
        ESS_TYPEDEF_FULL(ValueSeparatorNotFound, ParsingError);
        ESS_TYPEDEF_FULL(IllegalKeyName, ParsingError);
        ESS_TYPEDEF_FULL(DuplicateKeyFound, ParsingError);

        BfConfigParser(const StringList &config, bool caseSensitive = true); // can throw
        ~BfConfigParser();

        int GetVariablesCount() const;
        void GetVariable(int index, std::string &name, std::string &value) const;

        // Tests BfConfigParser
        static void Test();
    };


} // namespace BfConfig

#endif

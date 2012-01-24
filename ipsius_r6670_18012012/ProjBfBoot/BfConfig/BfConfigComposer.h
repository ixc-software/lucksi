#ifndef __BFCONFIGCOMPOSER__
#define __BFCONFIGCOMPOSER__


#include "Utils/ErrorsSubsystem.h"

#include "BfConfigCommon.h"


namespace BfConfig
{
    // ����� ��� ������������ ������������ ��� ����������
    class BfConfigComposer : private BfConfigCommon
    {
        std::vector<KeyValuePair> m_config;
        bool m_caseSensitive;

        std::string TryLowercase(const std::string &in) const;


    public:
        // ������ ������ ������������� �������
        ESS_TYPEDEF(IllegalKeyName);
        ESS_TYPEDEF(DuplicateKeyFound);

        BfConfigComposer(bool caseSensitive);
        ~BfConfigComposer();

        void AddVariable(const std::string& name, const std::string& value); // can throw
        void GetConfig(StringList &config);

        static void Test();
    };

} // namespace BfConfig


#endif

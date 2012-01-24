#ifndef __COMMANDDRIRESULT__
#define __COMMANDDRIRESULT__

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/StringList.h"

#include "driutils.h"

namespace DRI
{

    // basic abstract class
    class CommandDriResult
    {
        bool m_ok;
        Utils::StringList m_output;

    protected:

        CommandDriResult(bool ok, const Utils::StringList &output) :
             m_ok(ok), m_output(output)
         {
             // ...
         }

         CommandDriResult(const std::exception &e, const QString &addInfo = "") :
            m_ok(false)
         {            
            m_output.push_back( DriUtils::ExceptionToString(e, addInfo) );
         }

         static Utils::StringList StringToList(const QString &s)
         {
            Utils::StringList sl;
            if (!s.isEmpty()) sl.push_back(s);
            return sl;
         }

    public:

        virtual ~CommandDriResult() {}
       
        bool OK() const { return m_ok; }
        const Utils::StringList& Output() const { return m_output; }

        QString ErrorTypeinfo()
        {
            ESS_ASSERT(!OK());
            std::string s = Platform::FormatTypeidName( typeid(*this).name() );
            return s.c_str();
        }

        QString OutputAsString() const
        {
            QString res;

            for(int i = 0; i < m_output.size(); ++i)
            {
                res += m_output.at(i);
                if (i != (m_output.size() - 1)) res += "\n"; // C_LF;
            }

            return res;
        }

    };    
    
    // ------------------------------------------

    #define DRI_RESULT_TYPE(newTypeName, isOK)   \
        class newTypeName : public CommandDriResult \
        { \
        public: \
            newTypeName(const QString &s = "") : CommandDriResult(isOK, StringToList(s)) {} \
            newTypeName(const Utils::StringList &output) : CommandDriResult(isOK, output) {} \
            newTypeName(const std::exception &e,  const QString &addInfo = "") : CommandDriResult(e, addInfo) {} \
        };

    DRI_RESULT_TYPE(CommandDriResultOk, true);
    DRI_RESULT_TYPE(CommandDriTransactionError, false);
    DRI_RESULT_TYPE(CommandDriDiscarded, false);
    DRI_RESULT_TYPE(CommandDriPrepareError, false);
    DRI_RESULT_TYPE(CommandDriRuntimeError, false);
    DRI_RESULT_TYPE(CommandDriAborted, false);
    DRI_RESULT_TYPE(CommandDriRuntimeAsyncError, false);
    DRI_RESULT_TYPE(CommandDriWriteProperty, false);

    #undef DRI_RESULT_TYPE

    // ------------------------------------------

    // exception inside DRI commands
    ESS_TYPEDEF(ExecuteException);

    // exception during typecast string to arg type
    ESS_TYPEDEF(DriArgException);

    // extra params
    ESS_TYPEDEF(DriMethodExtraParams);

    // can't invoke
    ESS_TYPEDEF(InvokeException);

}  // namespace DRI

#endif


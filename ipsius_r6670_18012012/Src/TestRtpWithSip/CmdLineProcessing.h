#ifndef CMDLINEPROCESSING_H
#define CMDLINEPROCESSING_H

#include "Utils/ErrorsSubsystem.h"
#include "Utils/StringList.h"

namespace TestRtpWithSip
{    
    ESS_TYPEDEF(UserException);    

    class CmdProcessor
    {
        const Utils::StringList m_argList;
        //QString m_arg;
        //bool m_isCmd;

        //void RunCmd();
        void RunByFile();

    public:
        //CmdProcessor(int argc, char *argv[]);
        CmdProcessor(const Utils::StringList& argList);
        void Run();
    };

} // namespace TestRtpWithSip

#endif


#ifndef __DOMAINSTARTUPCONSTS__
#define __DOMAINSTARTUPCONSTS__

// DomainStartupConsts.h

#include "Utils/HostInf.h"
#include "DRI/DriIncludeDirList.h"

namespace Domain
{
    namespace DomainStartupDetails
    {
        struct Flags
        {
            // flags
            // domain
            static const char* DomainName() { return  "n"; }               // name 
            static const char* DomainName2() { return  "name"; }

            static const char* TelnetDriAddr() { return "t"; }             // host:port
            static const char* TelnetDriAddr2() { return "telnet"; }
            static const char* TelnetDriAddr3() { return "ta"; }            // old

            static const char* IncludeDirs() { return "i"; }               // "path1;path2"
            static const char* IncludeDirs2() { return "include"; }

            static const char* Password() { return "p"; }                  // password
            static const char* Password2() { return "password"; }

            static const char* SuppressVerbose() { return "s"; }           // <none>
            static const char* SuppressVerbose2() { return "silence"; }
            static const char* SuppressVerbose3() { return "sv"; }          // old

            // log
            static const char* RemoteLogAddr() { return "l"; }             // host:port
            static const char* RemoteLogAddr2() { return "log"; }
            static const char* RemoteLogAddr3() { return "la"; }            // old

            static const char* LogCmdResultTrim() { return "tr"; }         // charCount
            static const char* LogCmdResultTrim2() { return "trim_log"; }
            static const char* LogCmdResultTrim3() { return "lt"; }         // old

            // autorun
            static const char* AutoRunScriptFile() { return "sc"; }        // autorun file
            static const char* AutoRunScriptFile2() { return "script"; }
            static const char* AutoRunScriptFile3() { return "af"; }        // old

            static const char* EnableScriptTrace() { return "v"; }          // <none>
            static const char* EnableScriptTrace2() { return "verbose"; }
            static const char* EnableScriptTrace3() { return "at"; }         // old
            
            static const char* ExitAfterAutoRun() { return "e"; }          // <none> 
            static const char* ExitAfterAutoRun2() { return "exit"; }
            static const char* ExitAfterAutoRun3() { return "ae"; }         // old

            // disable exit
            static const char* DisableExitByKey() { return "nek"; }            // <none>
            static const char* DisableExitByKey2() { return "no_exit_key"; }
            static const char* DisableExitByKey3() { return "dk"; }             // old

            static const char* DisableExitOnAutorunError() { return "ne"; }    // <none>
            static const char* DisableExitOnAutorunError2() { return "ignore"; }
            static const char* DisableExitOnAutorunError3() { return "de"; }    // old

            // other
            static const char* SpecialParams() { return "x"; }         // params1 paramN
            static const char* SpecialParams2() { return "extra"; }
            static const char* SpecialParams3() { return "sp"; }        // old
        };

        // -----------------------------------------------

        struct Consts
        {
            static char AddrSep() { return  ':'; }
        };

        // -----------------------------------------------

        struct Defaults
        {
            static const char* DomainName() { return "Domain"; }
            
            static const char* TelnetDriAddr() { return ""; }
            static int TelnetDriPort() { return 0; }
    
            static const char* RemoteLogAddr() { return ""; }
            static int RemoteLogPort() { return 0; }
    
            static QString IncludeDirs() 
            {
                QString res("../isc/");
                res += DRI::DriIncludeDirList::PathSeparator();
                return res;
            }
                    
            static const char* AutoRunScriptFile() { return ""; }
            static bool EnableScriptTrace() { return false; }
            static bool ExitAfterAutoRun() { return false; }
            static bool SuppressVerbose() { return false; }
            static bool DisableExitByKey() { return false; }
            static const char* Password() { return ""; }
            static bool DisableExitOnAutorunError() { return false; }
            static int LogCmdResultTrim() { return -1; }
            
            /* no default values for Flags::SpecialParams */
        };

        // -----------------------------------------------
        
        
    } // namespace DomainStartupDetails
    
    
} // namespace Domain

#endif

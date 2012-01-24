#ifndef __TELNETOPTIONLIST__
#define __TELNETOPTIONLIST__

#include "Platform/PlatformTypes.h"
#include "TelnetUtils.h"

namespace Telnet
{
    // list of all enabled options on client or server side
    // also used for check sending mode(sending mode depends on ensbled option)
    class TelnetOptionList
    {
        class TelnetOpt
        {
            byte m_opt;             // | ECHO         | TTYPE
            bool m_enable;          // | false        | false
            bool m_requestInit;     // | false        | true (SB SEND TTYPE SE)
            bool m_requested;       // | false, if we must reply to DO or WILL 

        public:
            TelnetOpt(byte opt, bool requestInit = false) 
                : m_opt(opt), m_enable(false), 
                  m_requestInit(requestInit),
                  m_requested(false)
            {}

            void setOpt(byte opt) { m_opt = opt; }
            void setEnable(bool state) { m_enable = state; }
            void setRequestInit(bool state) { m_requestInit = state; }
            void setRequested(bool state) { m_requested = state; }

            byte getOpt() { return m_opt; }
            bool getEnable() { return m_enable; }
            bool getRequestInit() { return m_requestInit; }
            bool getRequested() { return m_requested; }
        };

        std::vector<TelnetOpt> m_list;

        TelnetOpt* Find(byte opt);

        bool AllModesDisable();

    public:
        TelnetOptionList();
        
        void Disable(byte opt);
        void Enable(byte opt);
        void SetRequested(byte opt, bool state = true);

        bool IsEnabled(byte opt);
        bool IsExists(byte opt);
        bool IsRequestInit(byte opt);
        bool IsRequested(byte opt);
        bool IsEmpty() { return m_list.empty(); }

        void Add(byte opt, bool requestInit = false);
        void Remove(byte opt);

        void Clear();

        size_t Size() { return m_list.size(); }

        SendingMode getOwnerSendingMode();
        bool SendingAsHalfDuplexTerminal(); // send CMD_GA

        ~TelnetOptionList();
    };

} // namespace

#endif


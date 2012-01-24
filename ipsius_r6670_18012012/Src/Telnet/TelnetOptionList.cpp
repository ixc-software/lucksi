
#include "stdafx.h"

#include "TelnetCommands.h"
#include "TelnetOptionList.h"

#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"

namespace Telnet
{
    TelnetOptionList::TelnetOptionList()
    {
        // true == request init (disabled all time)
        Add(OPT_TRANSMIT_BINARY);
        Add(OPT_ECHO);
        Add(OPT_SGA);
        Add(OPT_TTYPE, true); 
        Add(OPT_NAWS, true);
        Add(OPT_NEW_ENVIRON, true);
        Add(OPT_OLD_ENVIRON, true);
        // Add(OPT_TSPEED, true);
        Add(OPT_XDISPLOC, true);
        Add(OPT_EXTOPL);
        Add(XOPT_TRANSMIT_UNICODE);
        Add(XOPT_TRANSMIT_CODEC, true); 
        // ...
    }

    // ------------------------------------------------

    TelnetOptionList::~TelnetOptionList()
    {
        Clear();
    }

    // ------------------------------------------------

    TelnetOptionList::TelnetOpt* TelnetOptionList::Find(byte opt)
    {
        for (size_t i = 0; i < m_list.size(); ++i)
        {
            if (m_list.at(i).getOpt() == opt) return &m_list.at(i);
        }

        return 0;
    }

    // -----------------------------------------------

    void TelnetOptionList::Disable(byte opt)
    {
        if (!IsExists(opt)) return;

        if ((opt == OPT_NEW_ENVIRON)
            ||(opt == OPT_OLD_ENVIRON))
        {
            Find(OPT_NEW_ENVIRON)->setEnable(false);
            Find(OPT_OLD_ENVIRON)->setEnable(false);
            return;
        }
        
        Find(opt)->setEnable(false);
    }

    // -----------------------------------------------

    void TelnetOptionList::Enable(byte opt)
    {
        if (!IsExists(opt)) return;

        if ((opt == OPT_NEW_ENVIRON)
            ||(opt == OPT_OLD_ENVIRON))
        {
            Find(OPT_NEW_ENVIRON)->setEnable(true);
            Find(OPT_OLD_ENVIRON)->setEnable(true);
            return;
        }

        Find(opt)->setEnable(true);
    }

    // -----------------------------------------------

    void TelnetOptionList::SetRequested(byte opt, bool state)
    {
        if (!IsExists(opt)) return;
        Find(opt)->setRequested(state);
    }

    // -----------------------------------------------

    bool TelnetOptionList::IsExists(byte opt)
    {
        return (Find(opt) != 0) ? true : false;
    }

    // -----------------------------------------------

    bool TelnetOptionList::IsEnabled(byte opt)
    {
        return (IsExists(opt)) ? Find(opt)->getEnable() : false;
    }

    // -----------------------------------------------

    bool TelnetOptionList::IsRequested(byte opt)
    {
        return (IsExists(opt)) ? Find(opt)->getRequested() : false;
    }

    // -----------------------------------------------

    void TelnetOptionList::Add(byte opt, bool requestInit)
    {
        if (IsExists(opt)) return;

        m_list.push_back(TelnetOpt(opt, requestInit));
    }

    // -----------------------------------------------

    void TelnetOptionList::Remove(byte opt)
    {
        for (size_t i = 0; i < m_list.size(); ++i)
        {
            if (m_list.at(i).getOpt() == opt)
            {
                m_list.erase(m_list.begin() + i);
                return;
            }
        }
        
    }

    // ------------------------------------------------

    bool TelnetOptionList::IsRequestInit(byte opt)
    {
        return (IsExists(opt)) ? Find(opt)->getRequestInit() : false;
    }

    // ------------------------------------------------

    void TelnetOptionList::Clear()
    {
        m_list.clear();
    }

    // ---------------------------------------------

    bool TelnetOptionList::SendingAsHalfDuplexTerminal()
    {
        return (!IsEnabled(OPT_SGA)) ? true : false;
    }

    // ---------------------------------------------
    
    SendingMode TelnetOptionList::getOwnerSendingMode()
    {
        if (IsEnabled(OPT_TRANSMIT_BINARY)) return BinaryMode;

        if (IsEnabled(XOPT_TRANSMIT_UNICODE)) return UnicodeMode;

        return AsciiMode;
    }

} // namespace Telnet

#include "stdafx.h"
#include "ExitAction.h"
#include "Script.h"
#include "IReload.h"

namespace BfBootSrv
{
    void ExitAction::SetReload()
    {
        ESS_ASSERT(!m_exitAction && "Already set.");
        m_exitAction = new Utils::VIMethod<IReload>(&IReload::Reboot, m_hw);//new Utils::VIFunction(DevIpTdm::SwReboot);
    }

    void ExitAction::SetRunScript( Script& script )
    {
        ESS_ASSERT(!m_exitAction && "Already set.");        
        m_exitAction = new Utils::VIMethodWithRefParam<Script, IReload>(&Script::Run, script, m_hw);
        m_script = Utils::SafeRef<Script>(&script);
    }

    void ExitAction::Run()
    {    	
        if (!m_exitAction) return; // do nothing
        Utils::VIExecuter::Exec(m_exitAction);
        //m_exitAction->Execute();
        m_exitAction = 0;
        if (!m_script.IsEmpty()) m_script.Clear();
    }

    void ExitAction::Reset()
    {
        delete m_exitAction;
        m_exitAction = 0;
    }
} // namespace BfBootSrv


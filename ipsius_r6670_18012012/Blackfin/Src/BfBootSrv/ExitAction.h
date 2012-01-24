#ifndef EXITACTION_H
#define EXITACTION_H

#include "Utils/VirtualInvoke.h"
#include "Utils/SafeRef.h"


namespace BfBootSrv
{
    class Script;
    class IReload;

    // ��������� ������� �������� �������    
    class ExitAction : boost::noncopyable
    {              
        IReload& m_hw;
        // ��������� �� IVirtualInvoke � Script& ���������� � Run -- ?
        Utils::IVirtualInvoke* m_exitAction;
        Utils::SafeRef<Script> m_script;

    public:
        ~ExitAction(){ delete m_exitAction; }
        ExitAction(IReload& hw) : m_hw(hw), m_exitAction(0)
        {}

        void SetReload();
        void SetRunScript(Script& script);        
        void Run();
        void Reset();
    };
} // namespace BfBootSrv

#endif

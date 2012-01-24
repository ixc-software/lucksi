#include "stdafx.h"
#include "Executer.h"

#include "Utils/ErrorsSubsystem.h"
#include "Platform/Platform.h"

using namespace std;


namespace BfServer
{

    //-------------------------------------------------------------------------
    // ExecutionUnit implementation
    //-------------------------------------------------------------------------


    ExecutionUnit::ExecutionUnit(IExecuter &owner) :
        m_owner(owner)
    {
        owner.RegisterUnit(this);
    }


    //-------------------------------------------------------------------------

    
    ExecutionUnit::~ExecutionUnit()
    {
        m_owner.UnregisterUnit(this);
    }


    //-------------------------------------------------------------------------
    // IExecution unit implementation


    void ExecutionUnit::Execute()
    {
        OnExecute();
    }


    //-------------------------------------------------------------------------
    // Executer implementation
    //-------------------------------------------------------------------------


    Executer::Executer()
        : m_loopBreak(false)
    {
    }


    //-------------------------------------------------------------------------


    Executer::~Executer()
    {
        ESS_ASSERT(m_units.empty());
    }


    //-------------------------------------------------------------------------


    void Executer::SingleExecutionLoop()
    {
        vector<IExecutionUnit*>::iterator it = m_units.begin();

        m_listIsModified = false;

        for (; it != m_units.end(); ++it)
        {
            (*it)->Execute();
            if (m_listIsModified || m_loopBreak) break;
        }
    }


    //-------------------------------------------------------------------------


    void Executer::Execute()
    {
        ESS_ASSERT(true != m_units.empty());

        m_loopBreak = false;

        while (true)
        {
            SingleExecutionLoop();
            if (m_loopBreak) break;
        }
    }


    //-------------------------------------------------------------------------


    void Executer::BreakLoop()
    {
        m_loopBreak = true;
    }


    //-------------------------------------------------------------------------
    // IExecuter implementation

    void Executer::RegisterUnit(IExecutionUnit *pUnit)
    {
        ESS_ASSERT(0 != pUnit);
        ESS_ASSERT(std::find(m_units.begin(), m_units.end(), pUnit) == m_units.end());

        m_units.push_back(pUnit);

        m_listIsModified = true;
    }


    //-------------------------------------------------------------------------


    void Executer::UnregisterUnit(IExecutionUnit *pUnit)
    {
        ESS_ASSERT(0 != pUnit);

        std::vector<IExecutionUnit*>::iterator it = std::find(m_units.begin(), m_units.end(), pUnit);
        ESS_ASSERT(it != m_units.end());

        m_units.erase(it);

        m_listIsModified = true;
    }

} // namespace BfServer

#include "stdafx.h"

#include "DelayInit.h"

// ------------------------------------------

namespace Utils
{
       
    void DelayInitHost::Register( IVirtualInvoke *pInvoke )
    {
        ESS_ASSERT(pInvoke != 0);
        m_list.Add(pInvoke);
    }

    // ----------------------------------------------

    void DelayInitHost::DoInit()
    {
        ESS_ASSERT(!m_initDone);

        for(int i = 0; i < m_list.Size(); ++i)
        {
            m_list[i]->Execute();
        }

        m_list.Clear();
        m_initDone = true;
    }

    // ----------------------------------------------

    DelayInitHost& DelayInitHost::Inst()
    {
        static DelayInitHost host;
        return host;
    }


}  // namespace Utils


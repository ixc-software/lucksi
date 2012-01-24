#include "stdafx.h"
#include "L2Profiles.h"
#include "isdnl2.h"

namespace ISDN
{
    void L2Profile::Set(IsdnL2* pL2) const// override
    {
        IsdnL2::Layer2 *l2 = pL2->getL2();
        l2->m_flag.FIXED_TEI = m_fixedTei; // запрет назначения TEI
        l2->m_flag.ORIG = m_orig;
        l2->tei = m_tei;

        if (m_initialState1)
            l2->SetST_L2_1();
        else
            l2->SetST_L2_4();

        l2->ChangeMaxWindow(m_maxWindowSize);

        l2->m_isUserSide = m_isUserSide;// m_pRole = m_role;
    }
} // namespace ISDN


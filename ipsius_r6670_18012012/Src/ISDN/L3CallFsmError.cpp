#include "stdafx.h"
#include "L3CallFsmError.h"
#include "L3CallFsm.h"

namespace ISDN
{

    void ErrActionCloseCall::DoAction( L3CallFsm& call ) /*  */
    {
        call.DoCloseCall(m_num);
    }
} // namespace ISDN

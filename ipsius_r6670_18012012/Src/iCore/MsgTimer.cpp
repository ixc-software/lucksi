
#include "stdafx.h"
#include "MsgTimer.h"

// -----------------------------------------

namespace iCore
{


    namespace Detail
    {
        void MsgTimerDummyFn()
        {
            // nothing
        }
    }

    // ������ �� ���������������
    template class MsgTimerT<int>;

}  // namespace iCore

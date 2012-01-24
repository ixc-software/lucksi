
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

    // запрос на инстанцирование
    template class MsgTimerT<int>;

}  // namespace iCore

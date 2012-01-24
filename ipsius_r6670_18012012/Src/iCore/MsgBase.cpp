
#include "stdafx.h"
#include "Utils/AtomicTypes.h"
#include "Utils/ILockable.h"
#include "Utils/DelayInit.h"

#include "MsgBase.h"
#include "MsgObject.h"

// -------------------------------------------------------------

namespace
{
    Utils::AtomicInt Counter(0);
}

namespace iCore
{
    void MsgBase::CounterCheck()
    {
        ESS_ASSERT(Counter.Get() == 0);
    }

    MsgBase::MsgCounter::MsgCounter()
    {
        Counter.Inc();
    }

    MsgBase::MsgCounter::~MsgCounter()
    {
        Counter.Dec();
    }


    // ----------------------------------------------------------

    MsgBase::MsgBase(MsgObjectBase *pObject) : m_destination(pObject), m_singature(0)
    {
        ESS_ASSERT(pObject);
        if (CDoObjectCheck) m_singature = m_destination->GetSignature();
    }

    // ----------------------------------------------------------

    void MsgBase::SignatureCheck()
    {
        if (CDoObjectCheck && m_singature) m_destination->SignatureCheck(m_singature);
    }


    // ----------------------------------------------------------

    bool MsgBase::IsDestinationTo(MsgObjectBase *pObject)
    {
        return (m_destination == Utils::SafeRef<MsgObjectBase>(pObject));
    }

    // ----------------------------------------------------------

    void MsgBase::CheckDestination()
    {
        ESS_ASSERT(!m_destination->getDestroyRequested());
    }

    // ----------------------------------------------------------

    namespace
    {
        Utils::AtomicInt GThreadCounter;
        Utils::ILockable *GPLock = 0;
        Utils::DelayInit MutexInit(GPLock);

        enum
        {
            CCheckMsgCount = true,
        };

    }

    ThreadMsgCounterChecker::ThreadMsgCounterChecker()
    {
        ESS_ASSERT(GPLock != 0);
        Utils::Locker locker(*GPLock);
        GThreadCounter.Inc();
    }

    ThreadMsgCounterChecker::~ThreadMsgCounterChecker()
    {
        ESS_ASSERT(GPLock != 0);
        Utils::Locker locker(*GPLock);

        ESS_ASSERT(GThreadCounter.Get());
        GThreadCounter.Dec();

        if (GThreadCounter.Get() == 0)
        {
            if (CCheckMsgCount) MsgBase::CounterCheck(); 
        }
    }

    // ----------------------------------------------------------


}

#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "Utils/AtomicTypes.h"

#include "iCoreMsgObjectSafeDelete.h"

// ---------------------------------------------

namespace
{
    using namespace iCore;
    using namespace Utils;

    class TestObject : public MsgObject
    {   
        int m_counter;

        void OnMsg()
        {
            m_counter++;
        }

    public:
        TestObject(MsgThread &thread) : MsgObject(thread), m_counter(0) {}

        void SendMsg()
        {
            PutMsg(this, &TestObject::OnMsg);
        }
    };

    void DeleteFromAnotherThread()
    {
        MsgThread thread("DeleteFromAnotherThread", Platform::Thread::LowPriority, true);
        TestObject obj(thread);

        obj.SendMsg();
        obj.SendMsg();
        obj.SendMsg();

        // удаление происходит из другого потока, поэтому объект перед удалением отрабатывает
        // все сообщения -- потенциально очень опасная ситуация !
    }

    void DeleteFromSameThread()
    {
        // проверяется в iCoreAsyncTests, через MsgObject::AsyncDeleteSelf()
    }

}  // namespace

// ---------------------------------------------

namespace iCoreTests
{
    void iCoreMsgObjectSafeDelete()
    {
        DeleteFromAnotherThread();
        DeleteFromSameThread();
    }

} // namespace iCoreTests

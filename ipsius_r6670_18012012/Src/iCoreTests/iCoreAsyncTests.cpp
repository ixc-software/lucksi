#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "Utils/AtomicTypes.h"

#include "iCoreAsyncTests.h"

/*

       Класс-сообщение содержит ссылку на объект-получатель в виде SafeRef,
     поэтому удалять напрямую объект из метода-обработчика сообщения НЕЛЬЗЯ. 
       Для обхода этого ограничения используется механизм MsgThread::RunAsyncTask(),
     который позволяет выполнить код в контексте потока объекта, но не в теле
     его метода-обработчика. 

     Класс из примера, ObjectSelfDeleted, использует метод MsgObject::AsyncDeleteSelf()
     для корректного удаления самого себя. Если в методе ObjectSelfDeleted::OnTimer
     вызвать деструктор объекта, то SafeRef сообщения не даст завершиться деструктору
     ObjectSelfDeleted нормальным образом.

     Пример с классом ObjectDeleteByFlag более тонкий -- этот класс в обработчике сообщения 
     взводит флаг, который указывает другому потоку, что его нужно удалить.
     Класс использует метод MsgObject::AsyncSetReadyForDeleteFlag() для установки 
     флага вне своего метода. Если флаг установить напрямую, прямо в методе OnTimer,
     то всегда есть ненулевая вероятность попытки удаления объекта из другого потока 
     до того, как завершился метод OnTimer(), и возникнет проблема SafeRef сообщения, по которому
     и был вызван этот метод.


*/


// --------------------------------------------

namespace
{
    using namespace iCore;
    using namespace Utils;

    class ObjectSelfDeleted : public MsgObject
    {
        MsgTimer m_timer;
        AtomicBool &m_completedFlag;

        void OnTimer(MsgTimer *pTimer)
        {
            AsyncDeleteSelf();
            
            // delete this; // <- incorrect

            // проверка очистки сообщений в очереди, при удалении объекта из его же потока
            PutMsg(this, &ObjectSelfDeleted::Dummi);
            PutMsg(this, &ObjectSelfDeleted::Dummi);
            PutMsg(this, &ObjectSelfDeleted::Dummi);
        }

        void Dummi()
        {
        }

    public:

        ObjectSelfDeleted(MsgThread &thread, AtomicBool &completedFlag) : 
          MsgObject(thread),
          m_timer(this, &ObjectSelfDeleted::OnTimer), m_completedFlag(completedFlag)
        {
            m_timer.Start(50, false);
        }

        ~ObjectSelfDeleted()
        {
            m_completedFlag.Set(true);
        }
    };
	
    void DeleteSelfTest()
    {
        Utils::AtomicBool testCompleted(false);
        MsgThread thread("DeleteSelfTest", Platform::Thread::LowPriority, true);

        new ObjectSelfDeleted(thread, testCompleted);

        SyncTimer t;

        while(true)
        {
            if (t.Get() > 5000) TUT_ASSERT(0 && "Timeout");

            if (testCompleted.Get())
            {
                Platform::Thread::Sleep(1000);  // wait for ~ObjectSelfDeleted complete
                break;
            }
        }
    }
	
}  // namespace


// --------------------------------------------

namespace
{

    class ObjectDeleteByFlag : public MsgObject
    {
        MsgTimer m_timer;
        AtomicBool m_completedFlag;

        void OnTimer(MsgTimer *pTimer)
        {
            AsyncSetReadyForDeleteFlag(m_completedFlag);

            // m_completedFlag.Set(true); // <- incorrect
        }

    public:

        ObjectDeleteByFlag(MsgThread &thread) : 
          MsgObject(thread),
          m_timer(this, &ObjectDeleteByFlag::OnTimer), m_completedFlag(false)
        {
            m_timer.Start(50, false);
        }

        bool Completed()        
        {
            return m_completedFlag.Get();
        }

    };


    void DeleteByFlagTest()
    {
        MsgThread thread("DeleteByFlagTest", Platform::Thread::LowPriority, true);
        ObjectDeleteByFlag obj(thread);

        SyncTimer t;

        while(true)
        {
            if (t.Get() > 5000) TUT_ASSERT(0 && "Timeout");

            if (obj.Completed()) break;
        }
    }

}  // namespace

// --------------------------------------------

namespace iCoreTests
{
    void iCoreAsyncTests()
    {
        DeleteSelfTest();
        DeleteByFlagTest();
    }

} // namespace iCoreTests

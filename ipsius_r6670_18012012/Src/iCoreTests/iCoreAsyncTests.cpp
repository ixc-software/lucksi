#include "stdafx.h"
#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "Utils/AtomicTypes.h"

#include "iCoreAsyncTests.h"

/*

       �����-��������� �������� ������ �� ������-���������� � ���� SafeRef,
     ������� ������� �������� ������ �� ������-����������� ��������� ������. 
       ��� ������ ����� ����������� ������������ �������� MsgThread::RunAsyncTask(),
     ������� ��������� ��������� ��� � ��������� ������ �������, �� �� � ����
     ��� ������-�����������. 

     ����� �� �������, ObjectSelfDeleted, ���������� ����� MsgObject::AsyncDeleteSelf()
     ��� ����������� �������� ������ ����. ���� � ������ ObjectSelfDeleted::OnTimer
     ������� ���������� �������, �� SafeRef ��������� �� ���� ����������� �����������
     ObjectSelfDeleted ���������� �������.

     ������ � ������� ObjectDeleteByFlag ����� ������ -- ���� ����� � ����������� ��������� 
     ������� ����, ������� ��������� ������� ������, ��� ��� ����� �������.
     ����� ���������� ����� MsgObject::AsyncSetReadyForDeleteFlag() ��� ��������� 
     ����� ��� ������ ������. ���� ���� ���������� ��������, ����� � ������ OnTimer,
     �� ������ ���� ��������� ����������� ������� �������� ������� �� ������� ������ 
     �� ����, ��� ���������� ����� OnTimer(), � ��������� �������� SafeRef ���������, �� ��������
     � ��� ������ ���� �����.


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

            // �������� ������� ��������� � �������, ��� �������� ������� �� ��� �� ������
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

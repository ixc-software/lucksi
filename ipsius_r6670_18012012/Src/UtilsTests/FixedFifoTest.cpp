#include "stdafx.h"
#include "FixedFifoTest.h"
#include "Utils/FixedFifo.h"

namespace
{    
    class ObjType
    {
        int& m_objCounter;
    public:
        ObjType(int& objCounter)
            : m_objCounter(objCounter)
        {
            ++m_objCounter;
        }

        ~ObjType()
        {            
            --m_objCounter;
        }
    };
}

namespace UtilsTests
{
    using namespace Utils;

    void FixedFifoTest()
    {
        const int CMaxQueueSize = 10;
        FixedFifo<int*> queue(CMaxQueueSize);

        TUT_ASSERT(queue.IsEmpty()); // �����
        for (int i = 0; i < CMaxQueueSize; ++i)
        {
            TUT_ASSERT(!queue.IsFull()); // �� �����
            queue.Push(new int(i));
            TUT_ASSERT(queue.Size() == i + 1);
            TUT_ASSERT(!queue.IsEmpty()); // �� �����
        }

        TUT_ASSERT(queue.IsFull());

        for (int i = 0; i < CMaxQueueSize; ++i)
        {
            TUT_ASSERT (i == *queue.Pop()); // �������� ����������� ������������
        }
        TUT_ASSERT(queue.IsEmpty()); // �����

        // �������� �������� ���������� � ~FixedFifo 
        {
            int objCounter = 0;
            {
                FixedFifo<ObjType*> queue(CMaxQueueSize);
                queue.Push(new ObjType(objCounter));
                queue.Push(new ObjType(objCounter));
                queue.Clear();
                
                TUT_ASSERT(objCounter == 0); // �������� �������� ��� Clear()
                queue.Push(new ObjType(objCounter));                
                queue.Push(new ObjType(objCounter));
            }        
            
            TUT_ASSERT(objCounter == 0);        
        }

        // �������� Head
        {
            FixedFifo<int> queue(CMaxQueueSize);
            for (int i = 0; i < CMaxQueueSize; ++i)
            {
                queue.Push(i);
                TUT_ASSERT( queue.Head() == 0);
            }
            for (int i = 0; i < CMaxQueueSize; ++i)
            {                
                TUT_ASSERT( queue.Head() == i);
                queue.Pop();
            }
        }

    }
} // UtilsTests


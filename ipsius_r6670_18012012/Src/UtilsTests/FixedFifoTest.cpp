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

        TUT_ASSERT(queue.IsEmpty()); // пуста
        for (int i = 0; i < CMaxQueueSize; ++i)
        {
            TUT_ASSERT(!queue.IsFull()); // не полна
            queue.Push(new int(i));
            TUT_ASSERT(queue.Size() == i + 1);
            TUT_ASSERT(!queue.IsEmpty()); // не пуста
        }

        TUT_ASSERT(queue.IsFull());

        for (int i = 0; i < CMaxQueueSize; ++i)
        {
            TUT_ASSERT (i == *queue.Pop()); // проверка очередности выталкивания
        }
        TUT_ASSERT(queue.IsEmpty()); // пуста

        // проверка удаления указателей в ~FixedFifo 
        {
            int objCounter = 0;
            {
                FixedFifo<ObjType*> queue(CMaxQueueSize);
                queue.Push(new ObjType(objCounter));
                queue.Push(new ObjType(objCounter));
                queue.Clear();
                
                TUT_ASSERT(objCounter == 0); // проверка удаления при Clear()
                queue.Push(new ObjType(objCounter));                
                queue.Push(new ObjType(objCounter));
            }        
            
            TUT_ASSERT(objCounter == 0);        
        }

        // проверка Head
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


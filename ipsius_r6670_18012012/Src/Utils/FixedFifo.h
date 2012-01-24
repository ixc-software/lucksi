#ifndef FIXEDFIFO_H
#define FIXEDFIFO_H

#include "DeleteIfPointer.h"

namespace Utils
{   

    // hide copy constructor and operator =
    // we can't use boost::noncopyable 
    class FixedFifoBase
    {
    protected:
        FixedFifoBase() {}
        FixedFifoBase(const FixedFifoBase &other) {}
        void operator = (const FixedFifoBase &other) {}
    };
	
    // Fifo-buffer use heap only in constructor. Run on fixed in constructor region.
    template<class TStored>
    class FixedFifo : FixedFifoBase 
    {
        const int m_maxSize;
        int m_head; // out
        int m_tail; // in
        int m_maxIndex;        
        int m_objCount;
        int m_peakCount;  // max m_objCount during work
        bool m_manageIfPtr;
        std::vector<TStored> m_store;

        void IncIndex(int& index)
        {
            if (++index > m_maxIndex) index = 0;
        }

        TStored& StoreAt(int indx)
        {
            ESS_ASSERT( indx < m_store.size() );  // remove this for perfomance 
            return m_store[indx];  
        }
        const TStored& StoreAt(int indx) const 
        {
            ESS_ASSERT( indx < m_store.size() );  // remove this for perfomance 
            return m_store[indx];  
        }

    public:
        /*
        depth - fixed capacity
        manageIfPtr - delete stored object if it are pointer in ~FixedFifo()
        */
        FixedFifo(int depth, bool manageIfPtr = true) : 
            m_maxSize(depth),
            m_head(0),
            m_tail(0),
            m_maxIndex(depth - 1),
            m_objCount(0),
            m_peakCount(0),
            m_manageIfPtr(manageIfPtr),
            m_store(depth)
        {
            ESS_ASSERT(depth > 0);
        }

        ~FixedFifo()
        {            
            Clear();
        }

        // it's can be really dangerous if TStored is pointer
        void CopyInto(FixedFifo<TStored> &other) const
        {
            other = *this;
        }

        int MaxSize() const { return m_maxSize; }

        void Push(const TStored& obj)
        {
            ESS_ASSERT(!IsFull() && "Queue overflow!");
            StoreAt(m_tail) = obj;
            IncIndex(m_tail);
            ++m_objCount;
            if (m_objCount > m_peakCount) m_peakCount = m_objCount;
        }

        TStored Pop()
        {
            ESS_ASSERT(!IsEmpty() && "Pop nothing. FIFO is empty!");            
            
            TStored rez = StoreAt(m_head);
            IncIndex(m_head);
            --m_objCount;
            return rez;
        }

        // первый на выходе элемент. состояние очереди не меняется
        TStored& Head() 
        {
            ESS_ASSERT(!IsEmpty() && "Pop nothing. FIFO is empty!"); 
            return StoreAt(m_head);
        }

        const TStored& Head() const 
        {
            ESS_ASSERT(!IsEmpty() && "Pop nothing. FIFO is empty!"); 
            return StoreAt(m_head);
        }


        bool IsEmpty() const
        {
            return m_objCount == 0;
        }

        bool IsFull() const
        {
            return m_objCount == (m_maxIndex + 1);
        }
        
        void Clear()
        {
            while(!IsEmpty())
            {
                TStored obj = Pop();
                if (m_manageIfPtr) DeleteIfPointer(obj);
            }                
        }

        int Size() const
        {
            return m_objCount;
        }
        
        int PeakCount() const { return m_peakCount; }

    };

    

} // namespace Utils

#endif


#ifndef __MANAGEDLIST__
#define __MANAGEDLIST__

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"

namespace Utils
{
    // class delete his elements-pointers by himself 
    template<class T>
    class ManagedList : public boost::noncopyable
    {        
        bool m_managed;
        bool m_allowNullItems;
        std::vector<T*> m_list;

        void DoDelete(size_t index, bool removeFromList)
        {
            ESS_ASSERT( IndexInRange(index) );

            if (m_managed) boost::checked_delete( m_list.at(index) );

            if (removeFromList) m_list.erase(m_list.begin() + index);
        }

        // delete items [from; to)
        void DeleteRange(size_t from, size_t to)
        {
            ESS_ASSERT(from < m_list.size());
            ESS_ASSERT(to <= m_list.size());
            ESS_ASSERT(from <= to);

            for(size_t i = from; i < to; ++i)
            {
                DoDelete(i, false);
            }

            m_list.erase(m_list.begin() + from, m_list.begin() + to);
        }

    public:

        ManagedList(bool managed = true, size_t reserveCount = 0, bool allowNullItems = false)
            : m_managed(managed), m_allowNullItems(allowNullItems)
        {
            Reserve(reserveCount);
        }

        ~ManagedList()
        {
            Clear();
        }

        bool IndexInRange(size_t index) const
        {
            return (index < m_list.size());
        }

        // work only in m_allowNullItems mode
        void AddEmptyItems(size_t count)
        {
            ESS_ASSERT(m_allowNullItems);
            // ESS_ASSERT(count > 0);

            Reserve(Size() + count);

            while(count--) m_list.push_back(0);
        }
    
        void Add(T* item)
        {
            if (!m_allowNullItems) ESS_ASSERT(item != 0);
            m_list.push_back(item);
        }

        T* Front()
        {
            ESS_ASSERT(!m_list.empty());
            return m_list.front();
        }

        T* Back()
        {
            ESS_ASSERT(!m_list.empty());
            return m_list.back();
        }
     
        void Reserve(size_t size)
        {
            m_list.reserve(size);
        }
    
        size_t Size() const
        {
            return m_list.size();
        }

        size_t Capacity() const
        {
            return m_list.capacity();
        }

        T* operator[](size_t index) const
        {
            return m_list.at(index);
        }

        // remove item from list and get ownership over it
        T* Detach(size_t itemIndex)
        {
            T *p = m_list.at(itemIndex);
            m_list.erase(m_list.begin() + itemIndex);
            return p;
        }

        bool IsEmpty() const
        {
            return m_list.empty();
        }

        void Delete(size_t index)
        {
            DoDelete(index, true);
        }

        void Delete(size_t pos, size_t count)
        {
            DeleteRange(pos, pos + count);
        }

        void Clear()
        {
            Resize(0);
        }

        void Resize(size_t newSize)
        {
            if (newSize == m_list.size()) return;

            if (newSize > m_list.size())  // grow
            {
                ESS_ASSERT(m_allowNullItems);

                m_list.reserve(newSize);

                size_t count = newSize - m_list.size();

                for(size_t i = 0; i < count; ++i)
                {
                    m_list.push_back(0);
                }
            }
            else  // shrink 
            {
                DeleteRange(newSize, m_list.size());
            }

            ESS_ASSERT(m_list.size() == newSize);
        }

        void Set(size_t indx, T *pNewItem)
        {
            ESS_ASSERT( IndexInRange(indx) );

            if (!m_allowNullItems) ESS_ASSERT(pNewItem != 0);
            if (m_managed) boost::checked_delete( m_list.at(indx) );

            m_list.at(indx) = pNewItem;
        }

        template<class Par> 
        int Find(const Par *p) const 
        {
            for (size_t i = 0; i < m_list.size(); ++i)
            {
                if (m_list.at(i) == p) return i;
            }

            return -1;
        }

        // move all items to other list
        void MoveTo(ManagedList<T> &other)
        {
            ESS_ASSERT(other.m_managed        == m_managed);
            ESS_ASSERT(other.m_allowNullItems == m_allowNullItems);
            ESS_ASSERT( other.IsEmpty() );

            other.m_list = m_list;
            m_list.clear();
        }

    };

    
} // namespace Utils

#endif

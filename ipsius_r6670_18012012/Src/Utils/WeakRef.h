#ifndef __WEAKREF__
#define __WEAKREF__

#include "ThreadContext.h"
#include "IBasicInterface.h"

namespace Utils
{

    namespace detail
    {

        class IWeakRef : public Utils::IBasicInterface
        {
        public:
            virtual void Delete(const IWeakRef *pSender) = 0;
        };

    }  // namespace detail

    // ----------------------------------------------
    
    template<class T>
    class WeakRef
    {

        class Body : public detail::IWeakRef
        {
            detail::IWeakRef *m_pOwner;
            T m_value;

        // IWeakRef impl
        private:

            void Delete(const detail::IWeakRef *pSender)
            {
                ESS_ASSERT(m_pOwner == pSender);
                m_pOwner = 0;
            } 

        public:

            Body(T value, detail::IWeakRef &owner) : 
              m_pOwner(&owner),
              m_value(value)
            {
            }

            ~Body()
            {
                if (m_pOwner != 0)
                {
                    m_pOwner->Delete(this);
                }
            }

            T Value() const
            {
                ESS_ASSERT( Valid() );
                return m_value;
            }

            bool Valid() const
            {
                return (m_pOwner != 0);
            }


        };

        boost::shared_ptr< Body > m_body;
     
    public:

        WeakRef(T value, detail::IWeakRef &owner, 
                /* out */ detail::IWeakRef **body)
        {
            m_body.reset( new Body(value, owner) );
            *body = m_body.get();
        }

        // for container usage
        WeakRef()
        {
        }

        ~WeakRef()
        {
            // nothing
        }

        T Value() const
        {
            ESS_ASSERT(m_body != 0);
            return m_body->Value();
        }

        bool Valid() const
        {
            if (m_body == 0) return false;
            return m_body->Valid();
        }

        bool Empty() const
        {
            return !Valid();
        }

        bool operator== (const WeakRef<T> &other) const
        {
            if (Valid() && other.Valid())
            {
                return ( Value() == other.Value() );
            }

            return ( Empty() && other.Empty() ) ? true : false;
        }

        bool operator!= (const WeakRef<T> &other) const
        {
            return !(*this == other);
        }

    };

    // ----------------------------------------------

    class WeakRefHost : public detail::IWeakRef, boost::noncopyable
    {
        typedef std::list< detail::IWeakRef* > List;

        Utils::ThreadContext m_context;

        List m_binds;

    // IWeakRef impl
    private:

        void Delete(const detail::IWeakRef *pSender)
        {
            m_context.Assert();

            List::iterator i = std::find(m_binds.begin(), m_binds.end(), pSender);
            ESS_ASSERT(i != m_binds.end());
            m_binds.erase(i);
        }

    public:

        WeakRefHost()
        {
        }

        ~WeakRefHost()
        {
            Clear();
        }

        template<class T>
        WeakRef<T> Create(T value)
        {
            m_context.Assert();

            detail::IWeakRef *pBody = 0;
            WeakRef<T> result(value, *this, &pBody); 

            ESS_ASSERT(pBody != 0);
            m_binds.push_back(pBody);

            return result;
        }

        bool Empty() const
        {
            return m_binds.empty();
        }

        void Clear()
        {
            m_context.Assert();

            List::iterator i = m_binds.begin();

            while(i != m_binds.end())
            {
                detail::IWeakRef *p = *i;
                p->Delete(this);
                ++i;
            }

            m_binds.clear();
        }

    };

    // ----------------------------------------------

    void WeakRefTest();

    
}  // namespace Utils


#endif
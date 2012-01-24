#ifndef __BOOST_DETAIL_SHAREDCOUNTER__
#define __BOOST_DETAIL_SHAREDCOUNTER__

#include "boost/checked_delete.hpp"
#include "TestUtils.h"
#include "ThreadStrategy.h"
#include "Utils/ErrorsSubsystem.h"

namespace boost
{
	namespace detail
	{
        // Base interface for all pointer's wrappers
        class IPointerWrapperWithCounter
        {
        public:
            virtual void AddRefCopy() = 0;
			virtual bool Release() = 0;
			virtual long UseCount() const = 0;
            virtual ~IPointerWrapperWithCounter() {}
        };

        // ------------------------------------------------

		// Pointer wrapper with atomic counter, 
		// managed pointer deletion, when counter == 0
		template<class Y>
		class PointerWrapperWithCounter 
			: public IPointerWrapperWithCounter, 
			  noncopyable,
			  TestDestructor
		{
			static const bool CDebugMode = false;

			Y *m_pY;
			int m_use_count;
            
	    // IPointerWrapperWithcounter impl
		private:
            void AddRefCopy()
			{
                ThreadStrategy::Increment(&m_use_count);
			}

			bool Release() // nothrow
			{
                if (ThreadStrategy::FetchAndAdd(&m_use_count, -1) == 1)
                {
					boost::checked_delete(m_pY);

					if (CDebugMode) DebugStream() << "pointer released <-------------\n";

                    return true;
				}

                return false;
			}
		
            long UseCount() const // nothrow
			{
				return static_cast<int const volatile &>(m_use_count);
			}

		public:
			explicit PointerWrapperWithCounter(Y *p) 
			: TestDestructor("PointerWrapperWithcounter", CDebugMode), 
			  m_pY( p ), m_use_count(1)
			{
			}
		
			virtual ~PointerWrapperWithCounter() // nothrow
			{
			}
		};
        
		// ------------------------------------------------

		// Implement shared pointer's copying semantic
        class SharedCounter
			: TestDestructor
		{
			static const bool CDebugMode = false;

			IPointerWrapperWithCounter *m_pPtrWrapper;
			
			void TryRelease()
			{
				if (m_pPtrWrapper == 0) return;

				if (m_pPtrWrapper->Release()) delete m_pPtrWrapper;
				m_pPtrWrapper = 0;
			} 


		public:

			ESS_TYPEDEF(StdBadAllocation);
		
			SharedCounter()									// nothrow
			: TestDestructor("SharedCounter", CDebugMode), 
			  m_pPtrWrapper(0) 
			{
			}
		
			template<class Y> explicit SharedCounter(Y *p) 
			: TestDestructor("SharedCounter", CDebugMode), 
			  m_pPtrWrapper( 0 )
			{
                try
                {
                    m_pPtrWrapper = new PointerWrapperWithCounter<Y>(p);
                }
                catch(...)
                {
                    boost::checked_delete(p);
                    throw;
                }

                if (CDebugMode) DebugStream() << "tc:UseCount() = " << UseCount() << std::endl;
			}
		
			SharedCounter(const SharedCounter &other) 
			: TestDestructor("SharedCounter", CDebugMode), 
			  m_pPtrWrapper(other.m_pPtrWrapper) // nothrow
			{
                if( m_pPtrWrapper == 0 ) return;
				m_pPtrWrapper->AddRefCopy();

				if (CDebugMode) DebugStream() << "cc:UseCount() = " << UseCount() << std::endl;
			}

			~SharedCounter() // nothrow
			{
				TryRelease();                
			}

            SharedCounter &operator= (const SharedCounter &other) // nothrow
			{
				IPointerWrapperWithCounter *tmp = other.m_pPtrWrapper;

				// other == self --> return
				if(tmp == m_pPtrWrapper) return *this;

				// dec this's counter and delete ptr if needs				
				TryRelease();			
				
				// inc other's counter
				if( tmp != 0 ) tmp->AddRefCopy();

                m_pPtrWrapper = tmp;

				if (CDebugMode) DebugStream() << "ca:UseCount() = " << UseCount() << std::endl;

				return *this;
			}
		
			void Swap(SharedCounter &other) // nothrow
			{
				IPointerWrapperWithCounter *tmp = other.m_pPtrWrapper;
				other.m_pPtrWrapper = m_pPtrWrapper;
				m_pPtrWrapper = tmp;
			}
		
			long UseCount() const // nothrow
			{
				return (m_pPtrWrapper != 0) ? m_pPtrWrapper->UseCount() : 0;
			}
		
			bool Unique() const // nothrow
			{
				return UseCount() == 1;
			}
		
			bool Empty() const // nothrow
			{
				return m_pPtrWrapper == 0;
			}
		
			friend inline bool operator==(const SharedCounter &a, const SharedCounter &b)
			{
				return a.m_pPtrWrapper == b.m_pPtrWrapper;
			}
		
			friend inline bool operator<(const SharedCounter &a, const SharedCounter &b)
			{
				return std::less<IPointerWrapperWithCounter*>()(a.m_pPtrWrapper, 
																b.m_pPtrWrapper);
			}
		};
		
	} // namespace detail

} // namespace boost

#endif

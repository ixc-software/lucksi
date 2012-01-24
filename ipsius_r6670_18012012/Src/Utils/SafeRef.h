#ifndef __SAFEREF__
#define __SAFEREF__

#include "stdafx.h"
#include "Platform/Platform.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/AtomicTypes.h"

namespace Utils
{


	class BaseSafeRef;

	class ISafeRefOwner : public IBasicInterface
	{
	public:
		virtual void IncSafeRef(BaseSafeRef *pRef) const = 0;
		virtual void DecSafeRef(BaseSafeRef *pRef) const = 0;      
	};

    // ---------------------------------------------

    class BaseSafeRef
    {
    public:
        virtual std::string GetTypeInfo() const = 0;
        virtual ~BaseSafeRef() {}
    };

	// ---------------------------------------------
   
	template<class TIntf>
		class SafeRef : public BaseSafeRef
	{
		template<class T> friend class SafeRef;
		const ISafeRefOwner *m_pOwner;
		TIntf *m_pIntf;

		void TryBind()
		{
			if (m_pOwner) m_pOwner->IncSafeRef(this);         
		}

		void TryUnbind()
		{
			if (m_pOwner) m_pOwner->DecSafeRef(this);
		}
            
	public:

		template<class TOwner>
			SafeRef(TOwner *pOwner) : m_pOwner(pOwner), m_pIntf(pOwner)
		{
			TryBind();
		}

		SafeRef(ISafeRefOwner *pOwner, TIntf *pIntf) :
			m_pOwner(pOwner), m_pIntf(pIntf)
		{
			TryBind();
		}

		SafeRef() : m_pOwner(0), m_pIntf(0)
		{          
		}

		SafeRef(const SafeRef &other) : BaseSafeRef()
		{
			m_pOwner = other.m_pOwner; 
			m_pIntf = other.m_pIntf;

			TryBind();
		}

		template<class TIntfOther> 
			SafeRef(const SafeRef<TIntfOther> &other) : BaseSafeRef()
		{
			m_pOwner = other.m_pOwner; 
			m_pIntf = other.m_pIntf;

			TryBind();
		}
	
		~SafeRef()
		{
			TryUnbind();
		}

		std::string GetTypeInfo() const  // override
		{
		#ifdef COMPILER_FORWARD_TYPEID_ALLOWED
			return Platform::FormatTypeidName(typeid(TIntf).name());
		#else
			return "<unsupported>";
		#endif
		}

		void operator=(const SafeRef &other)
		{
			// free
			TryUnbind();
			// set new
			m_pOwner = other.m_pOwner; 
			m_pIntf = other.m_pIntf;
			TryBind();
		}

		template<class TIntfSrc> 
			void operator=(const SafeRef<TIntfSrc> &other)
		{
			// free
			TryUnbind();

			// set new
			m_pOwner = other.m_pOwner; 
			m_pIntf = other.m_pIntf;
			TryBind();
		}

		TIntf* operator->()
		{
			ESS_ASSERT(m_pIntf);
			return m_pIntf;
		}

		TIntf* operator->() const
		{
			ESS_ASSERT(m_pIntf);
			return m_pIntf;
		}

        TIntf& operator*() const
        {
            ESS_ASSERT(m_pIntf);
            return *m_pIntf;
        }

		bool operator == (const SafeRef<TIntf> &other) const
		{
			return (other.m_pIntf == m_pIntf);
		}

		bool operator != (const SafeRef<TIntf> &other) const
		{
			return !(other == *this);
		}

		bool IsEqualIntf(const TIntf *other) const
		{
			return other == m_pIntf;
		}
      
		bool IsEmpty() const
		{
			return m_pOwner == 0;
		}

		operator bool () const
		{
			return !IsEmpty();
		}

		TIntf* Clear()
		{
			ESS_ASSERT(!IsEmpty());

			TryUnbind();

			TIntf* p = m_pIntf;

			m_pOwner = 0; 
			m_pIntf = 0;      

			return p;
		}
	  
	/*      
		  // unsafe - !!
		  TIntf* get()
		  {
			  return m_pIntf;
		  } 
	*/   
	};




    // -------------------------------------------------------------

/*
   template<class TIntf> bool operator == (const SafeRef<TIntf> &ref, const TIntf *ptr) 
   {
       return ref.IsEqualIntf(ptr);
   };

   template<class TIntf> bool operator != (const SafeRef<TIntf> &ref, const TIntf *ptr) 
   {
       return !(ref == ptr);
   };

   template<class TIntf> bool operator == (const TIntf *ptr, const SafeRef<TIntf> &ref) 
   {
       return ref == ptr;
   };

   template<class TIntf> bool operator != (const TIntf *ptr, const SafeRef<TIntf> &ref) 
   {
       return !(ptr == ref);
   };
*/

    // -------------------------------------------------------------

    class SafeRefServerDebug;
   
    // класс надо наследовать вирутально - !!
    class SafeRefServer : public virtual ISafeRefOwner
    {
        mutable AtomicInt m_ref;
        mutable boost::shared_ptr<SafeRefServerDebug> m_debug;
        mutable std::string m_typeInfo;
        std::string m_objectName;

        // ISafeRefOwner implementation      
        void IncSafeRef(BaseSafeRef *pRef) const;
        void DecSafeRef(BaseSafeRef *pRef) const;

        void Init();

    public:

        ESS_TYPEDEF(DestructionInfo);

        SafeRefServer();

        SafeRefServer(const std::string &name);

        virtual ~SafeRefServer();

        void SetSafeRefServerName(const std::string &name)
        {
            m_objectName = name;
        }

        int getCountRef() const 
        {
            return m_ref;
        }
    };

    bool SafeRefDebugModeEnabled();
   

}  // namespace Utils

// --------------------------------

namespace UtilsTests
{
    void RunSafeRefTest(bool showInfo);
    
} // namespace UtilsTests

#endif


#ifndef __VIRTUALINVOKE__
#define __VIRTUALINVOKE__

/* Полиморфный (не шаблонный) аналог std::bind и boost::bind  */

#include "Utils/IBasicInterface.h"

namespace Utils
{

    class IVirtualInvoke : public Utils::IBasicInterface
    {
    public:
        virtual void Execute() = 0;
    };

    // Execute and delete instances of Utils::IVirtualInvoke
    class VIExecuter
    {
    public:
        static void Exec(IVirtualInvoke *pInvoke) 
		{ 
			boost::scoped_ptr<IVirtualInvoke> e(pInvoke); 
			e->Execute();
		}
    };


    // ---------------------------------------------------

    // impl for "void fn()"
    class VIFunction : public IVirtualInvoke
    {
    public:
        typedef void (*FnPtr)();

        VIFunction(FnPtr fn) : m_fn(fn) {}

    // IVirtualInvoke impl
    private:
        void Execute() { m_fn(); }

        FnPtr m_fn;
    };

	// ---------------------------------------------------
    template<class Fn>
	class VIFunctor : public IVirtualInvoke
	{
	public:

		VIFunctor(Fn fn) : m_fn(fn) {}

		// IVirtualInvoke impl
	private:
		void Execute() { m_fn(); }

		Fn m_fn;
	};


    // ---------------------------------------------------

    // impl for "void fn(T param)"
    template<class T>
    class VIFunctionWithParam : public IVirtualInvoke
    {
    public:
        typedef void (*FnPtr)(T);

        VIFunctionWithParam(FnPtr fn, T param) : m_fn(fn), m_param(param) {}

    // IVirtualInvoke impl
    private:
        void Execute() { m_fn(m_param); }

        FnPtr m_fn;
        T m_param;
    };

	// ---------------------------------------------------
	
    // impl for "void fn(T &param)"
    template<class T>
    class VIFunctionWithRefParam : public IVirtualInvoke
    {
    public:
        typedef void (*FnPtr)(T&);

        VIFunctionWithRefParam(FnPtr fn, T &param) : m_fn(fn), m_param(param) {}

    // IVirtualInvoke impl
    private:
        void Execute() { m_fn(m_param); }

        FnPtr m_fn;
        T &m_param;
    };

	// ---------------------------------------------------
	
    // impl for "void fn(const T &param)"
    template<class T>
    class VIFunctionWithConstRefParam : public IVirtualInvoke
    {
    public:
        typedef void (*FnPtr)(const T&);

        VIFunctionWithConstRefParam(FnPtr fn, const T &param) : m_fn(fn), m_param(param) {}

    // IVirtualInvoke impl
    private:
        void Execute() { m_fn(m_param); }

        FnPtr m_fn;
        T m_param;
    };
	
    // ---------------------------------------------------

    // impl for "void TBase::fn()"        
    template<class TBase>
    class VIMethod : public IVirtualInvoke
    {
    public:
        typedef void (TBase::*FnPtr)();

        VIMethod(const FnPtr fn, TBase &owner) : m_fn(fn), m_owner(owner) {}

    // IVirtualInvoke impl
    private:
        void Execute() { (m_owner.*m_fn)(); }

        FnPtr m_fn;
        TBase &m_owner;
    }; 

    // ---------------------------------------------------

    // impl for "void TBase::fn(T param)"        
    template<class TBase, class T>
    class VIMethodWithParam : public IVirtualInvoke
    {
    public:
        typedef void (TBase::*FnPtr)(T);

        VIMethodWithParam(FnPtr fn, TBase &owner, T param) 
            : m_fn(fn), m_owner(owner), m_param(param) {}

    // IVirtualInvoke impl
    private:
        void Execute() { (m_owner.*m_fn)(m_param); }

        FnPtr m_fn;
        TBase &m_owner;
        T m_param;
    }; 

	// ---------------------------------------------------

    // impl for "void TBase::fn(T &param)"        
    template<class TBase, class T>
    class VIMethodWithRefParam : public IVirtualInvoke
    {
    public:
        typedef void (TBase::*FnPtr)(T&);

        VIMethodWithRefParam(FnPtr fn, TBase &owner, T &param) 
            : m_fn(fn), m_owner(owner), m_param(param) {}

    // IVirtualInvoke impl
    private:
        void Execute() { (m_owner.*m_fn)(m_param); }

        FnPtr m_fn;
        TBase &m_owner;
        T &m_param;
    };

	// ---------------------------------------------------

    // impl for "void TBase::fn(const T &param)"        
    template<class TBase, class T>
    class VIMethodWithConstRefParam : public IVirtualInvoke
    {
    public:
        typedef void (TBase::*FnPtr)(const T&);

        VIMethodWithConstRefParam(FnPtr fn, TBase &owner, const T &param) 
            : m_fn(fn), m_owner(owner), m_param(param) {}

    // IVirtualInvoke impl
    private:
        void Execute() { (m_owner.*m_fn)(m_param); }

        FnPtr m_fn;
        TBase &m_owner;
        T m_param;
    };

    // ---------------------------------------------------

    // impl for "void TBase::fn(T &param1, const T &param2)"        
    template<class TBase, class T1, class T2>
    class VIMethodWithTwoParam : public IVirtualInvoke
    {
    public:
        typedef void (TBase::*FnPtr)(T1&, const T2&);

        VIMethodWithTwoParam(FnPtr fn, TBase &owner, T1 &param1, const T2& param2) 
            : m_fn(fn), m_owner(owner), m_param1(param1), m_param2(param2)
        {}

    // IVirtualInvoke impl
    private:
        void Execute() { (m_owner.*m_fn)(m_param1, m_param2); }

        FnPtr m_fn;
        TBase &m_owner;
        T1 &m_param1;
        T2 m_param2;
    };
    
    // ---------------------------------------------------

    class VirtualInvoke
    {
    public:

        static IVirtualInvoke* Create(VIFunction::FnPtr fn)
        {
            return new VIFunction(fn);
        }

        template<class TFunc>
        static IVirtualInvoke* Create(TFunc f)
        {
            return new VIFunctor<TFunc>(f);
        }

        template<class T>
        static IVirtualInvoke* Create(void (*pFn)(T), T param)
        {
            return new VIFunctionWithParam<T>(pFn, param);
        }

        template<class T>
        static IVirtualInvoke* Create(void (*pFn)(T&), T &param)
        {
            return new VIFunctionWithRefParam<T>(pFn, param);
        }

		template<class T>
        static IVirtualInvoke* Create(void (*pFn)(const T&), const T &param)
        {
            return new VIFunctionWithConstRefParam<T>(pFn, param);
        }
		
        template<class TBase>
        static IVirtualInvoke* Create(void (TBase::*pFn)(), TBase &owner)
        {
            return new VIMethod<TBase>(pFn, owner);
        }
        
        template<class TBase, class T>
        static IVirtualInvoke* Create(void (TBase::*pFn)(T), TBase &owner, T param)
        {
            return new VIMethodWithParam<TBase, T>(pFn, owner, param);
        }

		template<class TBase, class T>
        static IVirtualInvoke* Create(void (TBase::*pFn)(T&), TBase &owner, T &param)
        {
            return new VIMethodWithRefParam<TBase, T>(pFn, owner, param);
        }

		template<class TBase, class T>
        static IVirtualInvoke* Create(void (TBase::*pFn)(const T&), TBase &owner, const T &param)
        {
            return new VIMethodWithConstRefParam<TBase, T>(pFn, owner, param);
        }

        template<class TBase, class T1, class T2>
        static IVirtualInvoke* Create(void (TBase::*pFn)(T1&, const T2&), TBase &owner, 
                                      T1 &param1, const T2 &param2)
        {
            return new VIMethodWithTwoParam<TBase, T1, T2>(pFn, owner, param1, param2);
        }
    };


}  // namespace Utils

#endif


#ifndef SIMPLECMD_H
#define SIMPLECMD_H

#include "stdafx.h"
#include "MemoryPool.h"
#include "IExecutor.h"
#include "BaseSpecAllocated.h"

namespace Utils
{
    // execute external void(*fun)(TArg&) with argument
    template<class TArg>
    class Executor : boost::noncopyable, 
		public IExecutor,
        public Utils::BaseSpecAllocated<IFixedMemBlockPool>
    {
        typedef void(*TFn)(TArg&);

        TArg m_arg;
        TFn m_callBack;        

        void Execute()
        {
            m_callBack(m_arg);            
        }

    public:

        Executor( TFn callBack, const TArg& arg) : m_arg(arg),
            m_callBack(callBack)            
        {}

    };

    //---------------------------------------------------------------------------------------------

    // specialization for execute external void(*fun)()
    template<>
	class Executor<void> : boost::noncopyable, 
        public IExecutor,
        public Utils::BaseSpecAllocated<IFixedMemBlockPool>
    {
        typedef void(*TFn)();
        
        TFn m_callBack;        

        void Execute()
        {
            m_callBack();           
        }

    public:

        Executor( TFn callBack) 
            : m_callBack(callBack)            
        {}

    };

    //---------------------------------------------------------------------------------------------

    // execute member function of TOwner with argument
    template<class TOwner, class TArg>
	class MemExecutor : boost::noncopyable, 
        public IExecutor,
        public Utils::BaseSpecAllocated<IFixedMemBlockPool>
    {
        typedef void(TOwner::*Fn)(const TArg&);
        Fn m_fn;
        TArg m_arg;
        TOwner* m_pOwner;

        void Execute()
        {
            (m_pOwner->*m_fn)(m_arg);
        }

    public:
        MemExecutor(TOwner* pOwner, Fn callbackFn, const TArg& arg)
            : m_fn(callbackFn),
            m_arg(arg),
            m_pOwner(pOwner)
        {}
    };

    //---------------------------------------------------------------------------------------------

    //spec. for execute member function of TOwner without argument
    template<class TOwner>
	class MemExecutor<TOwner, void> : boost::noncopyable, 
        public IExecutor,
        public Utils::BaseSpecAllocated<IFixedMemBlockPool>
    {
        typedef void(TOwner::*Fn)();
        Fn m_fn;        
        TOwner* m_pOwner;

        void Execute()
        {
            (m_pOwner->*m_fn)();
        }

    public:
        MemExecutor(TOwner* pOwner, Fn callbackFn)
            : m_fn(callbackFn),            
            m_pOwner(pOwner)
        {}
    };

    //---------------------------------------------------------------------------------------------

    // создает возвращаая как void* и отрадатывает отложенные вызовы
    // нет аллокации из кучи
    template<bool CDeleteCmdInExecute = true, class TLockStrategy = DefaultThreadSafeStrategy>
    class ExecutorManager
    {
        FixedMemBlockPool<TLockStrategy> m_allocator;

        typedef IExecutor TIntf;

    public:        

        ExecutorManager(int msgSize, int capacity)
            : m_allocator(msgSize, capacity)
        {
            ESS_ASSERT (msgSize % Platform::CMemoryAlign == 0);
        }

        template<typename TArg>
        void* CreateExecutor(void(*callbackFn)(TArg&), const TArg& arg)
        {
            TIntf* pIntfExecute = new( m_allocator )Executor<TArg>(callbackFn, arg);
            return pIntfExecute;
        }
        
        void* CreateExecutor(void(*callbackFn)())
        {
            TIntf* pIntfExecute = new( m_allocator )Executor<void>(callbackFn);
            return pIntfExecute;
        }

        // todo owner is saferef server
        template<typename TOwner, typename TFn, typename TArg>
        void* CreateExecutor(TOwner* pOwner, TFn callbackFn, const TArg& arg)
        {
            TIntf* pIntfExecute = new( m_allocator )MemExecutor<TOwner, TArg>(pOwner, callbackFn, arg);
            return pIntfExecute;
        }

        template<typename TOwner, typename TFn>
        void* CreateExecutor(TOwner* pOwner, TFn callbackFn)
        {
            TIntf* pIntfExecute = new( m_allocator )MemExecutor<TOwner, void>(pOwner, callbackFn);
            return pIntfExecute;
        }
        
        static void Execute(void* executor)
        {
            ESS_ASSERT (executor != 0);

            TIntf* pExecutor = reinterpret_cast<TIntf*>(executor);
            pExecutor->Execute();

            if (CDeleteCmdInExecute) delete pExecutor;
        }     

        static void DeleteExecutor(void* executor)
        {
            TIntf* pExecutor = reinterpret_cast<TIntf*>(executor);                        
            delete pExecutor;
        }     

    };
} // namespace Utils

#endif

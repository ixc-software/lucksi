#ifndef __DELAYINIT__
#define __DELAYINIT__

#include "Platform/PlatformDelayInit.h"
#include "VirtualInvoke.h"
#include "ManagedList.h"

namespace Utils
{
    /*
        Эта система предназначена для возможности отложить инциализацию
        некоторых глобальных объектов.

        Регистрация отложенной инициализации делается с помощью класса DelayInit.
        Выполнение всех отложенных инициализаций с помощью DelayInitHost::DoInit.

        Система предназначена для решения проблемы на Blackfin -- там нет
        возможности в глобальных объектах создавать мютексы, т.к. инициализация
        VDK происходит после начала выполнения main().

    */

    class DelayInitHost
    {
        bool m_initDone;
        ManagedList<IVirtualInvoke> m_list;

    public:

        DelayInitHost() : m_initDone(false)
        {
        }

        void Register(IVirtualInvoke *pInvoke);
        void DoInit();

        static DelayInitHost& Inst();
    };
    

    class DelayInit
    {

        template<class T>
        class DelayCreate : public Utils::IVirtualInvoke
        {
            T *&m_p;

        // IVirtualInvoke impl
        private:

            void Execute()
            {
                ESS_ASSERT(m_p == 0);

                m_p = new T();
            }

        public:

            DelayCreate(T *&p) : m_p(p)
            {
                m_p = 0;
            }

        };

		static void DelayInvoke(IVirtualInvoke *pI)
		{
			ESS_ASSERT(pI != 0);

			if (Platform::CEnableDelayInit)
			{
				DelayInitHost::Inst().Register(pI);
			}
			else
			{
				pI->Execute();
				delete pI;
			}
		}


    public:

        template<class T>
        DelayInit(T *&p)
        {
            DelayInvoke(new DelayCreate<T>(p));
        };
		
		class Functor
		{
		public:
			template<class Fn> Functor(Fn fn)
			{
				DelayInvoke(new Utils::VIFunctor<Fn>(fn));
			}
		};

    };
    
    
    
}  // namespace Utils

#endif

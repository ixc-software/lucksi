
#ifndef __MSGBASE__

#define __MSGBASE__

#include "Utils/AtomicTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/SafeRef.h"
#include "MsgObjectBase.h"

namespace iCore
{

    class ThreadMsgCounterChecker
    {
    public:
        ThreadMsgCounterChecker();
        ~ThreadMsgCounterChecker();
    };

	// Базовый класс сообщения
	class MsgBase
	{
        class MsgCounter
        {
        public:
            MsgCounter(); 
            ~MsgCounter();
        };

        enum { CDoObjectCheck = true };

        struct DirectConver
        {
            static MsgObjectBase* Do(MsgObjectBase *p) { return p; }
        };

        struct DynamicConvert
        {
            template<class T>
            static MsgObjectBase* Do(T *p) 
            {                 
                MsgObjectBase *pObj = dynamic_cast<MsgObjectBase*>(p);
                ESS_ASSERT(pObj != 0);
                return pObj;
            }

        };

        Utils::SafeRef<MsgObjectBase> m_destination;
        int m_singature;

	protected:

		virtual void Execute() = 0;

        void CheckDestination();

        void SignatureCheck();

        // статическое или динамическое преобразование T* к MsgObjectBase*
        template<class T>
        static MsgObjectBase* ConvertToMsgObject(T *p)
        {
            const bool isMsgObject = boost::is_base_of<MsgObjectBase, T>::value;   
            typedef typename boost::mpl::if_c<isMsgObject, DirectConver, DynamicConvert>::type Tx;

            return Tx::Do(p);
        }

	public:

		MsgBase(MsgObjectBase *pObject);
        virtual ~MsgBase() {}
        
        virtual MsgBase* Clone() = 0;        // используется таймером        
        virtual std::string TypeName() = 0;  // используется в отладке

		void Process()
		{
            /*
                Мы перестаем "удерживать" через SafeRef получателя сообщения,
                это дает возможность объекту удалить самого себя в методе обработки сообщения
            */
            // if (!m_hold.Get()) m_destination.Clear();  

			Execute();
		}
        
        bool IsDestinationTo(MsgObjectBase *pObject);

        static void CounterCheck();
	};
}


#endif


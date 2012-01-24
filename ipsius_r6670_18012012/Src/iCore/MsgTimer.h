
#ifndef __MSGTIMER__


#define __MSGTIMER__

#include "stdafx.h"
#include "MsgBase.h"
#include "ITimerManager.h"
#include "ICheckObject.h"
#include "MsgTemplates.h"
#include "MsgObjectBase.h"

#include "Utils/DeleteIfPointer.h"
#include "Utils/ErrorsSubsystem.h"

namespace iCore
{

    /*
        TOwner через MsgThread должен выводить на инфраструктуру таймеров через GetTimerManager()
        MsgThread должны разделять эту инфраструктуру

        TOwner должен быть MsgObjectBase

    */

    namespace Detail
    {
        void MsgTimerDummyFn();
    }

    // Шаблон класса, генерирующего сообщение через заданный интервал
    template<typename TTag>
    class MsgTimerT : public IMsgTimer
    {
        enum { CMaxTimerInterval = 24 * 60 * 60 * 1000UL, };  
        enum { CMaxTimeout = 60 * 1000UL, };

        bool m_tagEmpty;
        TTag m_tag;
        boost::shared_ptr<MsgBase> m_event;
        MsgObjectBase *m_pOwner;
        ITimerManager *m_pManager;

        Utils::AtomicBool m_active;
        dword m_startTime;
        dword m_interval;
        bool m_repeat;
        bool m_blockEvent;

	private:

		template <class Owner> void InitOwner(Owner *owner) 
		{
			ESS_ASSERT(owner != 0);

			m_pOwner = owner;
			m_pManager = &(m_pOwner->GetTimerManager());
		}

		template<typename TOwner> void Init(TOwner *owner) 
		{
			InitOwner(owner) ;

			m_tagEmpty = true;
			m_tag = TTag();
			m_startTime = 0;
			m_interval = 0;
			m_repeat = false;
			m_blockEvent = false;

			// проверка включения в проект .cpp файла, в котором инстанцируется шаблон MsgTimer
			Detail::MsgTimerDummyFn();  
		}

        template<class T1, class T2, class T3>
        MsgBase* MakeEvent(T1 owner, T2 fn, T3 param)
        {
            return new MsgParam<T1, T2, T3>(owner, fn, param, true);		
        }

        void CheckFatalTimeout(dword elapsed)
        {
            if (elapsed < CMaxTimeout) return;  // OK

            std::ostringstream ss;
            ss << "MsgTimerT fatal timeout; start " << m_startTime 
                << "; interval " << m_interval 
                << "; current " << Platform::GetSystemTickCount();

            ESS_HALT( ss.str() );
        }

        void SendEvent()
        {
            // получатель сообщения был удален (нас уведомили через NotifyMsgObjectDestroyed)
            ESS_ASSERT(!m_blockEvent);  

            ESS_ASSERT(m_event.get() != 0);
            ESS_ASSERT(m_pOwner);

            m_pOwner->PutMsgInQueue(m_event->Clone());
        }

        void AssertNotActive()
        {
            /*
            if (m_active)
            {
                static int i = 0;
                i++;
            } */

            ESS_ASSERT(!m_active);
        }

    // IMsgTimer implementation
    private:

        void SafeStop() 
        {
            m_active.Set(false);
        }

        void SafeStart(dword currTicks, dword interval, bool repeated)  
        {
            m_active.Set(true);
            m_startTime = currTicks;
            m_interval = interval;
            m_repeat = repeated;
        }

        dword GetWaitTime(dword currTicks) 
        {
            ESS_ASSERT(m_active);

            dword elapsed = currTicks - m_startTime;

            if (elapsed > m_interval)  // timer done
            {
                CheckFatalTimeout(elapsed - m_interval);
                return 0;
            }

            return (m_interval - elapsed);
        }

        bool Process(dword currTicks) 
        {
            if (GetWaitTime(currTicks) > 0) return false;

            SafeStop();
            SendEvent();

            return true;
        }

        bool TryRestart(dword currTicks) 
        {
            if (!m_repeat) return false;

            m_active.Set(true);
            m_startTime = currTicks;

            return true;
        }

        void NotifyMsgObjectDestroyed(MsgObjectBase *pObject)
        {
            if (m_event.get() == 0) return;

            m_blockEvent = (m_event->IsDestinationTo(pObject));
        }

		void BindEvent(MsgBase *msg)
		{
			AssertNotActive();

			m_event.reset(msg);
		}

    public:

        template<typename TOwner, typename TFn>
        MsgTimerT(TOwner *owner, TFn fn) 
        {
            Init(owner); 

			BindEvent(owner, fn);
        }

		template<typename TOwner>
		MsgTimerT(TOwner *owner, MsgBase *msg)
		{
			Init(owner); 

			BindEvent(owner, msg);
		}

        virtual ~MsgTimerT()        
        {
            Stop();
            ClearTag();
        }

        template<typename TOwner, typename TFn>
        void BindEvent(TOwner *owner, TFn fn)
        {
			BindEvent(owner, MakeEvent(owner, fn, this));
        }

		template<typename TOwner>
		void BindEvent(TOwner *owner, MsgBase *msg)
		{
			ESS_ASSERT(owner != 0);

			m_pOwner = owner;
			
			BindEvent(msg);
		}

        TTag getTag()
        {
            ESS_ASSERT(!m_tagEmpty);
            return m_tag;
        }

        void setTag(TTag value)
        {
            m_tagEmpty = false;
            m_tag = value;
        }

        void ClearTag()
        {
            if (!m_tagEmpty) Utils::DeleteIfPointer(m_tag);
        }

		bool Enabled() const 
		{
			return m_active;
		}
        void Start(dword interval, bool repeated = false)
        {
            ESS_ASSERT(interval < CMaxTimerInterval);
            ESS_ASSERT(interval > 0);
            m_pManager->TimerStart(this, interval, repeated);
        }

        void Stop()
        {
            m_pManager->TimerStop(this);
        }


    };

    typedef MsgTimerT<int> MsgTimer;

	
}  // namespace iCore


#endif


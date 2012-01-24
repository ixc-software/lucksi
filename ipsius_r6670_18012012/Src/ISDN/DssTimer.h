#ifndef DssTimer_H
#define DssTimer_H

#include "stdafx.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"
#include "iCore/MsgThread.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"
#include "IIsdnL3Internal.h"

namespace ISDN
{   
    // интерфейс таймеров используемых в Dss
    class IDssTimer : public Utils::IBasicInterface
    {
    public:
        virtual void Stop() = 0;
        virtual void Start() = 0;
        virtual void Restart() = 0;
        virtual int GetExpiredCount() = 0;
    };

    //--------------------------------------------------------------------------

    // интерфейс стопера группы таймеров
    class IStopDssTimersGroup : public Utils::IBasicInterface
    {
    public:
        virtual void Reg(IDssTimer*) = 0;
        virtual void Unreg(IDssTimer*) = 0;
    };

    //--------------------------------------------------------------------------
    
    // таймер - генерирует вызов назначеной функции владельца с передачей указанного событием
    template<typename TOwner, typename TEvent>
    class DssTimer :
        public iCore::MsgObject,        
        public IDssTimer,        
        boost::noncopyable
    {
        typedef void(TOwner::*TFn)(const TEvent&);         
        
        const bool m_ignor;
        bool m_active;
        iCore::MsgTimer m_t;
        const dword m_interval;
        const int m_maxRepeat;

        int m_expCounter;

        TOwner* m_pOwner;
        TFn m_action;
        IStopDssTimersGroup& m_stoper;

        void Expire(iCore::MsgTimer*)
        {
            // така€ ситуаци€ возможна если это сообщение было в очереди на момент Stop
            if (!m_active) return;            

            if (m_maxRepeat != -1 && m_expCounter > m_maxRepeat)            
            {                
                Stop();
                return;
            }

            ++m_expCounter;
            (m_pOwner->*m_action)( TEvent(this) ); // execute action                    
        }

    public:
        
        DssTimer(IIsdnL3Internal& iL3, IStopDssTimersGroup& stoper, TOwner* pOwner,
                    TFn action , const DssTimerValue& val);

        ~DssTimer();

        bool IsIgnored() const
        { 
            return m_ignor;
        }

        bool IsActive() const
        {
            return m_active;
        }

        //ignore if already started
        void Start()
        {
            if (m_active) return; 

            if (!m_ignor)            
            {
                m_t.Start(m_interval, true);
                m_active = true;
            }
        }

        void Stop()
        {
            if (!m_ignor)
            {
                m_t.Stop();                 
                m_expCounter = 0;
                m_active = false;
            }
        }

        void Restart()
        {
            if (!m_ignor)
            {
                Stop();
                Start();
            }
        }

        // при первом срабатывании вернет 1
        int GetExpiredCount()
        {
            ESS_ASSERT(m_active); // при неактивном не имеет смысла
            return (m_ignor ? 0 : m_expCounter);
        }

        // сколько срабатываний осталось; -1 если не ограниченно
        int getRestRepeatCount() const 
        {
            if (m_maxRepeat == -1) return -1;
            return m_maxRepeat - m_expCounter;
        }

    };

    //--------------------------------------------------------------------------

    // функтор позвол€ющий остановить все таймера принадлежащие TOwner (стопер группы таймеров)
    class StopDssTimersGroup : public IStopDssTimersGroup
    {
        typedef std::set<IDssTimer*> SetTimers;
        SetTimers m_pTimers;       
    
        void Reg(IDssTimer* pTimer)
        {
            m_pTimers.insert(pTimer);
        }

        void Unreg(IDssTimer* pTimer)
        {            
            m_pTimers.erase(pTimer);
        }

    public:

        void operator()()
        {
            std::for_each(m_pTimers.begin(), m_pTimers.end(), std::mem_fun( &IDssTimer::Stop ) );
        }
    };

    //--------------------------------------------------------------------------

    template<class TOwner, class TEvents>
    DssTimer<TOwner, TEvents>::DssTimer(IIsdnL3Internal& iL3, IStopDssTimersGroup& stoper, TOwner* pOwner,
        TFn action , const DssTimerValue& val)
        : MsgObject( iL3.GetThread() ),           
        m_ignor(!val.m_inited ),
        m_active(false),
        m_t(this, &DssTimer::Expire),
        m_interval(val.m_interval),
        m_maxRepeat(val.m_maxRepeat),
        m_expCounter(0),
        m_pOwner(pOwner),
        m_action(action),
        m_stoper(stoper)
    {        
        if (!m_ignor)
        {
            ESS_ASSERT(pOwner);
            ESS_ASSERT(m_interval > 0  && m_maxRepeat != 0);
            stoper.Reg(this);
        }
    }

    //--------------------------------------------------------------------------

    template<class TOwner, class TEvents>
    DssTimer<TOwner, TEvents>::~DssTimer()
    {
        if (!m_ignor)
        {
            m_stoper.Unreg(this);
        }
    }
    

} // ISDN

#endif


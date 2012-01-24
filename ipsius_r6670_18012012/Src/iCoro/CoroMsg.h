#pragma once

#include "Utils/IBasicInterface.h"
#include "Utils/ThreadContext.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"

#include "dri/driutils.h"

#include "Coro.h"

namespace iCoro
{

    class CoroMsg : public iCore::MsgObject, boost::noncopyable
    {
        typedef CoroMsg T;

        enum State
        {
            stInitial, 
            stRunning,      // inside fiber 
            stWaiting,      // fiber suspended 
            stCompleted,
        };

        struct WaitMode
        {
            int TimeoutMode;
            bool WakeOnMsg;

            WaitMode() : TimeoutMode(0), WakeOnMsg(false)
            {                
            }

            WaitMode(int timeoutMode, bool wakeOnMsg) : 
                TimeoutMode(timeoutMode), WakeOnMsg(wakeOnMsg)
            {
            }
        };

    public:

        class IMsg : public Utils::IBasicInterface 
        {
        public:

            std::string MsgName() const
            {
                return DRI::DriUtils::FormatedTypename( typeid(*this).name(), true );
            }
        };

    protected:

        enum TimeoutMode
        {
            tmNoWait    = 0,
            tmDefault   = -1,
            tmUnlimited = -2,
        };

        typedef boost::shared_ptr<IMsg> SptrMsg;

        ESS_TYPEDEF(RuntimeError);
        ESS_TYPEDEF_T(ExceptionHook, SptrMsg);

        class MsgWaitTimeout : public IMsg
        {
        public:
        };

        class IMsgHook : public Utils::IBasicInterface
        {
        public:
            virtual bool ProcessMsg(const SptrMsg &msg) = 0;
        };

        template<class TMsg> 
        class MsgHook : public IMsgHook
        {
            enum { CCheck = boost::is_base_of<IMsg, TMsg>::value }; 
            BOOST_STATIC_ASSERT(CCheck);

            typedef boost::function<void (const boost::shared_ptr<TMsg>&)> MsgHookFn;

            CoroMsg &m_owner;
            MsgHookFn m_fn;

        // IMsgHook impl
        private:

            bool ProcessMsg(const SptrMsg &msg)
            {
                ESS_ASSERT(msg != 0);
                IMsg *p = msg.get();
                if ( typeid(*p) != typeid(TMsg) ) return false;

                boost::shared_ptr<TMsg> castedMsg = boost::dynamic_pointer_cast<TMsg>(msg);
                ESS_ASSERT(castedMsg != 0);
                m_fn(castedMsg);

                return true;
            }

        public:

            MsgHook(CoroMsg &owner, const MsgHookFn &fn) : 
              m_owner(owner), m_fn(fn)
            {
                ESS_ASSERT(m_fn);
                m_owner.RegisterMsgHook(this);
            }

            ~MsgHook()
            {
                m_owner.UnregisterMsgHook(this);
            }
        };

        template<class TMsg> 
        class MsgHookThrow
        {
            typedef MsgHookThrow<TMsg> T;

            MsgHook<TMsg> m_hook;

            void Callback(const boost::shared_ptr<TMsg> &msg)
            {
                SptrMsg baseMsg(msg);
                ESS_THROW_T(ExceptionHook, baseMsg);
            }

        public:
            MsgHookThrow(CoroMsg &owner) : 
              m_hook( owner, boost::bind(&T::Callback, this, _1) )
            {
            }
        };

        struct CoroCompletedInfo
        {
            std::string ExceptionTypeName;
            std::string ExceptionText;

            CoroCompletedInfo() 
            {
            }

            bool OK() const
            {
                return ExceptionTypeName.empty();
            }

            std::string ToString() const
            {
                if ( OK() ) return "OK";

                return "Exception " + ExceptionTypeName + "; " + ExceptionText;
            }
        };

        void PutMsg(IMsg *pMsg)
        {
            ESS_ASSERT(pMsg != 0);

            PutMsg( SptrMsg(pMsg) );
        }

        void PutMsg(const SptrMsg &msg)
        {
            ESS_ASSERT(msg != 0);

            if ( !m_thContext.Current() )
            {
                // call this method in own context 
                PutFunctor( boost::bind(&T::CallPutMsg, this, msg) );
                return;
            }

            // push 
            m_queue.push(msg);

            // resume - ?
            ESS_ASSERT(m_state == stWaiting);
            if (m_waitMode.WakeOnMsg) 
            {   
                // or use direct resume - ?!
                PutFunctor( boost::bind(&T::FiberContinue, this) );
            }
        }

        void CoroRun()
        {
            ESS_ASSERT(m_state == stInitial);
            m_thContext.Assert();

            FiberContinue();
        }

        void SetDefaultTimeout(int timeoutMs)
        {
            CheckTimeoutVal(timeoutMs);
            m_defaultTimeout = timeoutMs;
        }

        SptrMsg GetMsg(int timeoutMs = tmDefault)
        {
            ESS_ASSERT(m_state == stRunning);                
            m_thContext.Assert();

            if (timeoutMs == tmDefault) timeoutMs = m_defaultTimeout;
            CheckTimeoutVal(timeoutMs);

            SptrMsg msg = FiberReturn( WaitMode(timeoutMs, true) );

            if (msg != 0)
            {
                for(int i = 0; i < m_msgHooks.size(); ++i)
                {
                    bool captured = m_msgHooks.at(i)->ProcessMsg(msg);
                    if (captured) break;
                }
            }

            return msg;
        }

        template<class TMsg>
        boost::shared_ptr<TMsg> GetMsg(int timeoutMs = tmDefault)
        {
            const bool CCheck = boost::is_base_of<IMsg, TMsg>::value;
            BOOST_STATIC_ASSERT(CCheck);

            SptrMsg msg = GetMsg(timeoutMs);
            ESS_ASSERT(msg != 0);

            boost::shared_ptr<TMsg> res = boost::dynamic_pointer_cast<TMsg>(msg);
            if (res == 0)
            {
                std::string errMsg = "Can't conver msg ";
                errMsg += typeid( *(msg.get()) ).name();
                errMsg += std::string(" to ") + typeid(TMsg).name();
                ESS_THROW_MSG(RuntimeError, errMsg);
            }

            return res;
        }

        void Wait(int periodMs)
        {
            ESS_ASSERT(m_state == stRunning);                
            m_thContext.Assert();

            ESS_ASSERT(periodMs > 0);
            FiberReturn( WaitMode(periodMs, false) );
        }

        template<class TMsg>
        TMsg* MsgAs(const SptrMsg &msg)
        {
            IMsg *p = msg.get();
            ESS_ASSERT(p != 0);

            TMsg *pCasted = dynamic_cast<TMsg*>(p);

            return pCasted;
        }

        void Abort()
        {
            m_coro.Abort();
           
            // empty queue
            while( m_queue.size() ) m_queue.pop();
        }

        virtual void CoroBody() = 0;  // fiber context 
        virtual void CoroCompleted(const CoroCompletedInfo &info) = 0;

        CoroMsg(iCore::MsgThread &thread) : 
            iCore::MsgObject(thread),
            m_timer(this, &T::OnTimer),
            m_coro( boost::bind(&T::Body, this) ),
            m_state(stInitial),
            m_defaultTimeout(tmUnlimited)
        {
        }

        ~CoroMsg()
        {            
            Abort();  // assert -> must be completed - ?
        }

    private:

        SptrMsg FiberReturn(const WaitMode &mode)
        {
            ESS_ASSERT(m_state == stRunning);

            // already have message on queue
            if (mode.WakeOnMsg && m_queue.size())
            {
                return PopMsg();
            }

            // check queue without wait 
            if (mode.TimeoutMode == tmNoWait)
            {
                ESS_ASSERT( mode.WakeOnMsg );                
                return m_queue.size() ? PopMsg() : SptrMsg(); 
            }

            // enter wait mode 
            m_waitMode = mode;
            m_state    = stWaiting;

            if (mode.TimeoutMode > 0) 
            {
                m_timer.Start( mode.TimeoutMode );
            }

            m_coro.Return();   // return to main thread

            // awaken (after FiberContinue())
            // ... 

            return PopMsg();
        }

        void FiberContinue()
        {
            ESS_ASSERT(m_state != stRunning);

            m_timer.Stop();

            m_state = stRunning;
            m_coro.Run();           // switch to fiber

            // return from fiber to main thread till
            // ... 

            if (m_state == stCompleted)
            {
                PutFunctor( boost::bind(&T::CoroCompleted, this, m_completeInfo) );
            }
        }

        void Body()  // fiber context 
        {            
            ESS_ASSERT(m_state == stRunning);

            try
            {
                CoroBody();
            }
            catch(const std::exception &e)
            {
                m_completeInfo.ExceptionTypeName = typeid(e).name();
                m_completeInfo.ExceptionText     = e.what();

                {
                    const ExceptionHook *pE = dynamic_cast<const ExceptionHook*>(&e);
                    if (pE)
                    {
                        m_completeInfo.ExceptionText = "Hook for " + pE->getProperty()->MsgName();
                    }
                }
            }

            m_state = stCompleted;
        }

        void OnTimer(iCore::MsgTimer *pT)
        {
            if (m_state != stWaiting) return;
            PutMsg( new MsgWaitTimeout() );
        }

        static void CheckTimeoutVal(int timeoutMs)
        {
            bool ok = (timeoutMs > 0) || (timeoutMs == tmNoWait) || (timeoutMs == tmUnlimited);
            ESS_ASSERT(ok);
        }

        SptrMsg PopMsg()
        {
            ESS_ASSERT( m_queue.size() );

            SptrMsg m = m_queue.front();
            m_queue.pop();
            return m;
        }

        void RegisterMsgHook(IMsgHook *h)
        {
            m_thContext.Assert();

            ESS_ASSERT(h != 0);
            m_msgHooks.push_back(h);
        }

        void UnregisterMsgHook(IMsgHook *h)
        {
            m_thContext.Assert();

            ESS_ASSERT( m_msgHooks.removeAll(h) == 1);
        }

        void CallPutMsg(const SptrMsg &msg)
        {
            PutMsg(msg);
        }


        Utils::ThreadContext m_thContext; 
        iCore::MsgTimer m_timer;        
        Coro m_coro;        

        State m_state; 
        std::queue<SptrMsg> m_queue;   // don't use mutex - use context detectin + msg passing - ?!
        CoroCompletedInfo m_completeInfo;
        WaitMode m_waitMode;  // current wait mode (stWaiting)
        int m_defaultTimeout;
        QList<IMsgHook*> m_msgHooks;
    };
    
    
}  // namespace iCoro


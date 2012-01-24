
#ifndef __MSGOBJECTEXAMPLE__


#define __MSGOBJECTEXAMPLE__

#include "MsgObject.h"
#include "MsgTimer.h"

namespace iCore
{

    // Пример потомка MsgObject
	class MsgObjectExample : public MsgObject
	{
        typedef MsgObjectExample T;

        enum
        {
            CTimerInterval = 100,
        };

        MsgTimer m_timer;

        void OnThreadPoolReady()
        {
            // ...
        }

        void TheadPoolTask()
        {
            // do something in context of service thread (in ThreadPool)
            // ...

            // signal to object "task completed"
            PutMsg(this, &T::OnThreadPoolReady);
        }
      
		void OnSendEvent()
		{
			// do something as processing message in object thread context
            // ...
		}

        void OnTimer(MsgTimer *pT)
        {
            // this method work as callback for timer
            
            // schedule task for ThreadPool
            PutTask(this, &T::TheadPoolTask);
        }

	public:

		MsgObjectExample(MsgThread &thread) 
            : MsgObject(thread), m_timer(this, &T::OnTimer)
		{ 
            m_timer.Start(CTimerInterval, true);
		}

		void SendEvent()
		{
			PutMsg(this, &T::OnSendEvent);
		}
	  	
	};
 
		
}  // namespace


#endif 

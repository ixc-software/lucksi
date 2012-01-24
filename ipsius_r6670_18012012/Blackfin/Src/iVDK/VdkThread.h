#ifndef __VDKTHREAD__
#define __VDKTHREAD__

#include "VDK.h"
#include "iVDK/VdkUtils.h"
#include "iVDK/VdkCheckInit.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/IBasicInterface.h"

namespace iVDK
{


    namespace detail
    {
        class IThreadBody : public Utils::IBasicInterface
        {
        public:
            virtual void ThreadBody() = 0;
        };

        class IThreadIntf : public Utils::IBasicInterface
        {
        public:
            virtual void ClassThreadRun() = 0; 
            virtual int ClassThreadErrorHandler() = 0;
            virtual void ClassThreadDestroyed() = 0;
        };
       
    }  // namespace detail

    // -----------------------------------------------------------------

    class Thread : public detail::IThreadIntf, boost::noncopyable
    {
        enum State
        {
            StInit,
            StRunning,
            StDone,
        };

        State m_state; 
        VDK::ThreadID m_id;
        detail::IThreadBody &m_body;

        // 'static' VDK thread type data
        VDK::ThreadTemplate m_threadTempl;
        VDK::ThreadCreationBlock m_threadBlock;

         static VDK::ThreadID BadThreadID()
         {
             return static_cast<VDK::ThreadID>(UINT_MAX);
         }

         static VDK::Priority ConvertPriority(int pri)
         {
             // inherited mode
             if (pri < 0) return VDK::GetPriority( VDK::GetThreadID() );

             return static_cast<VDK::Priority>(pri);
         }

    // IThreadIntf impl
    private:

        void ClassThreadRun()
        {
			m_body.ThreadBody();
        }

        int ClassThreadErrorHandler()
        {
			// return VDK::Thread::ErrorHandler();
			
			ESS_HALT("Method unimpl");
			
			return 0;
        }

        void ClassThreadDestroyed()
        {
            m_state = StDone;
        }


    public:

        Thread(detail::IThreadBody &body) : 
            m_state(StInit),
        	m_id( BadThreadID() ), 
        	m_body(body)
        {
			VdkCheckInit();        	
        }

        ~Thread()
        {
            ESS_ASSERT(m_state != StRunning);
        }

        static int GetCurrentThreadID()
        {
            return VDK::GetThreadID();
        }

        static void Sleep(int msTime)
        {
            iVDK::Sleep(msTime);
        }

        static void CurrentThreadPriority(int priority)
        {
            VDK::SetPriority( VDK::GetThreadID(), ConvertPriority(priority) );
        }


        bool IsRunning() const
        {
            return (m_state == StRunning);
        }

        bool IsFinished() const
        {
            return (m_state == StDone);
        }


        void Wait()
        {
            // use sync primitive - ?

            while( IsRunning() )
            {
                Sleep(1);
            }
        }

        void SetPriority(int priority)
        {
            ESS_ASSERT( IsRunning() );

            CurrentThreadPriority(priority);
        }

        void Start(int priority);

    };

	
}  // namespace iVDK

#endif


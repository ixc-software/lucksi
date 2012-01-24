#include "stdafx.h"
#include "VdkThread.h"
#include "VDK.h"
#include "Utils/ErrorsSubsystem.h"

// -------------------------------------------------------

using iVDK::detail::IThreadIntf;

namespace
{
	
    enum
    {
        CThreadStackSideDW = 16 * 1024,
    };

    class ThreadClass : public VDK::Thread 
    {
        IThreadIntf *m_pIntf;

    public:

        ThreadClass(VDK::Thread::ThreadCreationBlock &block) : VDK::Thread(block)
        {
            m_pIntf = static_cast<IThreadIntf*>(block.user_data_ptr);
            ESS_ASSERT(m_pIntf != 0);
        }

        virtual ~ThreadClass()
        {
            m_pIntf->ClassThreadDestroyed();

            // Is it destroy on end run in 'idle' thread - ?
        }

        virtual void Run()
        {
        	try
        	{
        		m_pIntf->ClassThreadRun();
        	}
        	catch(/* const */ std::exception &e)
        	{
        		ESS_UNEXPECTED_EXCEPTION(e);
        	}
            
        }

        virtual int ErrorHandler()
        {
        	return VDK::Thread::ErrorHandler();
        	
            // return m_pIntf->ClassThreadErrorHandler();
        }

        static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock &block)
        {
            return new(block) ThreadClass(block);
        }
    };

}  // namespace

// -------------------------------------------------------

namespace iVDK
{

    void Thread::Start(int pri)
    {
        ESS_ASSERT( m_state == StInit );  // first start

        // priority
        VDK::Priority priority = ConvertPriority(pri);

        // template
        {
            VDK::ThreadTemplate	templ[] = 
            {
                INIT_THREADTEMPLATE_("iCoreThreadClass", priority, CThreadStackSideDW,  
                                     ::ThreadClass::Create, ksystem_heap, ksystem_heap, false)
            }; 

            m_threadTempl = templ[0];
        }
       
        // block
        IThreadIntf *pIntf = this;
        
        m_threadBlock.template_id = kDynamicThreadType;
        m_threadBlock.thread_stack_size = CThreadStackSideDW;
        m_threadBlock.thread_priority = priority;  
        m_threadBlock.user_data_ptr = pIntf;
        m_threadBlock.pTemplate = &m_threadTempl;

        // exec
        m_id = VDK::CreateThreadEx(&m_threadBlock); 
        ESS_ASSERT( m_id != BadThreadID() );

        m_state = StRunning;
    }


}  // namespace iVDK



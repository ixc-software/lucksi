#ifndef ADIGPTIMER_H
#define ADIGPTIMER_H

#include "Utils/ExecutorManager.h"
//#include "Utils/MemoryPoolForceDefaultInstantiation.h"
#include <services/services.h>

namespace BfDev
{
    using Platform::dword;
    
    // представление сервиса adi_tmr_GPXXX    
    class AdiGPTimer
    {        
        
        typedef Utils::ExecutorManager<false/*default treadsafe*/> ExecutorMng;
        
        const int CTimerID;
                
        ExecutorMng m_executorMng;
        void* m_currExecutor;
                       
        void Activate(dword period);
        
        /*void Reset()
        {           
            m_callbackIsSet = false; 
            ESS_ASSERT(
        	    adi_tmr_Reset(CTimerID) == ADI_TMR_RESULT_SUCCESS // сбрасывает callBack`и ???
        	    );
        }*/ 
        
        void AddCallback(void *pExecutor);
        
        static void OnInterrupt(void *executor, u32, void *);                       
  
    public:

        AdiGPTimer(int timerID, int maxExecutorSize = 36);

        ~AdiGPTimer();
    
        /*template<int Id>
        static AdiGPTimer& Instance()
        {
            BOOST_STATIC_ASSERT(ADI_TMR_GP_TIMER_COUNT > Id);                        
            int adiTimerID = ADI_TMR_CREATE_GP_TIMER_ID(Id);
            
            static AdiGPTimer tmr(adiTimerID);
            return tmr;
        }*/
    
        template<class Tfn, class TArg>
        void Start(Tfn fn, const TArg& arg, int period)
        {                       
            AddCallback(m_executorMng.CreateExecutor(fn, arg));    	        	    
    	    Activate(period);    	        	    
        }

        //-------------------------------------------------------------------
        
        template<class TOwner, class TFn, class TArg>
        void Start(TOwner owner, TFn fn, const TArg& arg, int period)
        {                       
            AddCallback(m_executorMng.CreateExecutor(owner, fn, arg));    	        	    
    	    Activate(period);    	        	    
        }

        //-------------------------------------------------------------------
        
        template<class TOwner, class TFn>
        void Start(TOwner owner, TFn fn, int period)
        {            
            AddCallback(m_executorMng.CreateExecutor(owner, fn, arg));    	        	    
    	    Activate(period);    	        	    
        }

        //-------------------------------------------------------------------
        
        void Stop();
        
    };
    
} // namespace BfDev

#endif

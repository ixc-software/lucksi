#include "stdafx.h"
#include "AdiGPTimer.h"

namespace
{
    class TimerIdRegister
    {
        std::set<int> m_reg;
        TimerIdRegister(){}
    public:
        static TimerIdRegister& Instance()
        {
            static TimerIdRegister registr;
            return registr;
        }

        void RegId(int id)
        {
            if (m_reg.insert(id).second != true)
                ESS_HALT("Timer with id as this already exist in system!");
        }

        void UnregId(int id)
        {
            std::set<int>::iterator i = m_reg.find(id);
            if (i == m_reg.end())
                ESS_HALT("Try to unreregistrate id witch not exist in registr");
        }
    };
    
} // namespace

namespace BfDev
{    

    

    AdiGPTimer::AdiGPTimer( int timerID, int maxExecutorSize ) : CTimerID(timerID),           
        m_executorMng(maxExecutorSize, 1),
        m_currExecutor(0)
    {
        TimerIdRegister::Instance().RegId(timerID);

        ADI_TMR_RESULT tmrOperationReport;

        tmrOperationReport = adi_tmr_Init(NULL);
        ESS_ASSERT(tmrOperationReport == ADI_TMR_RESULT_SUCCESS);                               

        ADI_TMR_GP_CMD_VALUE_PAIR timerCfg [] = {
            { ADI_TMR_GP_CMD_SET_TIMER_MODE,			(void *)0x01			},
            { ADI_TMR_GP_CMD_SET_COUNT_METHOD,			(void *)true			},
            { ADI_TMR_GP_CMD_SET_INTERRUPT_ENABLE,		(void *)true			},
            { ADI_TMR_GP_CMD_SET_OUTPUT_PAD_DISABLE,	(void *)true			},        		
            { ADI_TMR_GP_CMD_END,						0					    }, 
        };	


        tmrOperationReport = adi_tmr_Open(CTimerID);        
        ESS_ASSERT(tmrOperationReport == ADI_TMR_RESULT_SUCCESS);        

        tmrOperationReport = adi_tmr_GPControl(CTimerID, ADI_TMR_GP_CMD_TABLE, timerCfg);		
        ESS_ASSERT(tmrOperationReport == ADI_TMR_RESULT_SUCCESS);
    }

    //---------------------------------------------------------------------------

    AdiGPTimer::~AdiGPTimer()
    {
        ESS_ASSERT(adi_tmr_Close(CTimerID) == ADI_TMR_RESULT_SUCCESS);
        m_executorMng.DeleteExecutor( m_currExecutor );
        TimerIdRegister::Instance().UnregId(CTimerID);
    }

    //---------------------------------------------------------------------------

    void AdiGPTimer::Activate( dword period )
    {
        //dword period = 0x08000000; ///  каких едениц ????
        dword pulseWidth = period / 20;


        ADI_TMR_GP_CMD_VALUE_PAIR timerCmd[] = {                
            { ADI_TMR_GP_CMD_SET_PERIOD,                (void *)period          },
            { ADI_TMR_GP_CMD_SET_WIDTH,                 (void *)pulseWidth      },    		    
            { ADI_TMR_GP_CMD_ENABLE_TIMER,              (void *)true            },
            { ADI_TMR_GP_CMD_END,						0					    }, 
        };


        ESS_ASSERT(
            adi_tmr_GPControl(CTimerID, ADI_TMR_GP_CMD_TABLE, timerCmd) == ADI_TMR_RESULT_SUCCESS
            );
    }

    //---------------------------------------------------------------------------

    void AdiGPTimer::AddCallback(void *pExecutor)
    {
        ESS_ASSERT(m_currExecutor == 0); 
        m_currExecutor = pExecutor;

        bool wakeUpFlag = true;

        ESS_ASSERT(
            adi_tmr_InstallCallback(CTimerID, wakeUpFlag, m_currExecutor, 0, AdiGPTimer::OnInterrupt) == ADI_TMR_RESULT_SUCCESS 
            );
    }

    //---------------------------------------------------------------------------

    void AdiGPTimer::OnInterrupt( void *executor, u32, void* )
    {
        ExecutorMng::Execute(executor);
    }

    //---------------------------------------------------------------------------

    void AdiGPTimer::Stop()
    {
        ESS_ASSERT(m_currExecutor != 0);             

        ESS_ASSERT(
            adi_tmr_RemoveCallback(CTimerID) == ADI_TMR_RESULT_SUCCESS 
            );           

        m_executorMng.DeleteExecutor( m_currExecutor );
        m_currExecutor = 0;
    }

} // namespace BfDev

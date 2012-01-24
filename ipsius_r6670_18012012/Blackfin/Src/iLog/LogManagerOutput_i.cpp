#include "stdafx.h"
#include "LogManagerOutput.h"
#include "LogWrapperLibrary.h"
#include "LogUartStream_i.h"

namespace iLogW
{
    
    void LogOutputManager::TraceToFileOn(boost::shared_ptr<ILogRecordFormater> formater)
    {
    	ESS_HALT("Unsupported on blackfin");    	
    }
    

    // -----------------------------------------------------------------------

    void LogOutputManager::TraceToUartOn(boost::shared_ptr<ILogRecordFormater> formater)
    {
		m_settings.Uart().TraceInd = true;
		ClearStream(m_uartStream);

        m_uartStream.reset(new LogUartStream(m_settings.Uart().Port, formater));
        m_logOutput.Register(m_uartStream.get());
    }

};


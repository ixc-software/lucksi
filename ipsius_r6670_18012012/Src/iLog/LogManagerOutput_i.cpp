#include "stdafx.h"
#include "LogWrapperLibrary.h"
#include "LogOutSettings.h"
#include "LogManagerOutput.h"

namespace iLogW
{
    void LogOutputManager::TraceToFileOn(boost::shared_ptr<ILogRecordFormater> formater)
    {
        ESS_ASSERT(m_settings.File().NameLogFile.size() != 0 && "Filename mast be present");
        
		m_settings.File().TraceInd = true;
        ClearStream(m_fileStream);
        if (m_settings.File().DirectSaveToFile)
            m_fileStream.reset(new LogDirectFileStream(m_settings.File().NameLogFile, formater));
        else
            m_fileStream.reset(new LogFileStream(m_settings.File().NameLogFile, formater));
        m_logOutput.Register(m_fileStream.get());
    }

    // -----------------------------------------------------------------------

    void LogOutputManager::TraceToUartOn(boost::shared_ptr<ILogRecordFormater> formater)
    {
        ESS_HALT("Unsupported on PC");
    }
};


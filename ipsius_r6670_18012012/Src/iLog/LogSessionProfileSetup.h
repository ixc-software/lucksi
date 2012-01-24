#ifndef _LOG_SESSION_PROFILE_SETUP_H_
#define _LOG_SESSION_PROFILE_SETUP_H_

#include "stdafx.h"
#include "LogSessionSettings.h"

namespace iLogW
{
    class ILogRecordFormater;
    class LogSessionProfile;

    class LogSessionProfileSetup
    {
    public:        
        LogSessionProfileSetup(const LogSessionSettings &settings,
            boost::shared_ptr<ILogRecordFormater> formater);

        LogSessionProfileSetup(const LogSessionSettings &settings);

        ~LogSessionProfileSetup();

        LogSessionProfile & getLogSessionProfile();       
        
    private:
        boost::shared_ptr<ILogRecordFormater> m_formater;
        class Impl;
        Impl *m_impl;
    };
};

#endif

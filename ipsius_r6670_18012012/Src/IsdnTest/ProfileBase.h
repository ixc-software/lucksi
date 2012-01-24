#ifndef PROFILEBASE_H
#define PROFILEBASE_H

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/MetaConfig.h"
#include "Utils/ExeName.h"
#include "Utils/QtHelpers.h"

#include "iLog/LogWrapper.h"
#include "iLog/LogSessionSettings.h"

namespace IsdnTest 
{
    using Utils::MetaConfig;    
    using Utils::ExeName;
    using Utils::MetaConfigLoader;    

    
    class ProfileBase
    {
        MetaConfig m_config;
        iLogW::LogStore& m_logStore;
        iLogW::LogSessionSettings m_logOutProf;

        void InitDefault()
        {
            m_logOutProf.setCountSyncroToStore(100);
            //m_logOutProf.setLogFileName()
            m_logOutProf.setOutputToCout(false);
            m_logOutProf.setTimeoutSyncroToStore(500);
            m_logOutProf.setTimestampInd(true);            
        }

    public:

        ProfileBase(iLogW::LogStore& store)
            : m_logStore(store),
            m_logOutProf(m_config)
        {
            InitDefault();
        }

        ESS_TYPEDEF(WrongParametrs);
        
        virtual ~ProfileBase() {}        

        iLogW::LogStore& getLogStore() const
        {            
            return m_logStore;
        }

        const iLogW::LogSessionSettings& getLogOutSettings() const
        {
            return m_logOutProf;
        }        
       
        void LoadFromLoader(const MetaConfigLoader& loader)
        {
            if (!m_config.getCompleted()) 
            {
                AddFields(m_config);
                m_config.Complete();
            }

            m_config.LoadFrom(loader);
            ValidateFields();
        }

        QString GetInfo() const
        {
            return m_config.ListAllFields();
        }

        void setFile(const std::string& fileName)
        {
            m_logOutProf.setLogFileName(fileName);            
        }

    protected:

        virtual void ValidateFields() = 0;// can throw UserException
        //{
        //    // nothing todo
        //}

        virtual void AddFields(MetaConfig &config) = 0;

        void SaveToFile(const QString& cfgName)
        {
            AddFields(m_config);
            m_config.Complete();

            std::string fileName(ExeName::GetExeDir());
            fileName += Utils::QStringToString(cfgName);

            m_config.SaveToFile(fileName.c_str());
        }        

    };
} // namespace IsdnTest

#endif 



#ifndef TESTPROFILE_H
#define TESTPROFILE_H

#include "stdafx.h"
#include "Utils/HostInf.h"
#include "Utils/MetaConfig.h"
#include "iLog/LogSettings.h"
#include "iRtp/RtpPayload.h"
#include "iRtp/RtpParams.h"
#include "Sip/Codec.h"
#include "Sip/ProfileSip.h"

namespace TestRtpWithSip
{    
    class TestProfile
    {        
    // public types definition;
    public:
        ESS_TYPEDEF(WrongParametrs);        

        TestProfile();
        void SaveProfile(const std::string &cfgName);
        void LoadFromLoader(const Utils::MetaConfigLoader& loader); // can throw
        std::string ReflectHelp() const;

        const Sip::ProfileSip &getSipProfile() const {return m_sip;}
        int getDurationPlayDefaultFile() const {return m_durationPlayDefaultFile;}
        QString getAssignedFileName() const 
        {
            return m_assignedFileName;
        }
        QString getDefaultFileName() const 
        {
            return "DefaultRecFile.bin";
        }
        bool IsScnWritingToDefaultFile(const QString& userName) const
        {
            return userName == m_writingDefaultFile;
        }
        bool IsScnPlayDefaultFile(const QString& userName) const
        {
            return userName == m_playDefaultFile;
        }
        bool IsScnPlayDefaultFileWithTimer(const QString& userName) const
        {
            return userName == m_playDefaultFileWithTimer;
        }
        bool IsScnPlayAssignedFile(const QString& userName) const
        {
            return userName == m_playAssignedFile;
        }
        bool IsScnGenerator(const QString& userName) const
        {
            return userName.contains(CGenKey);
        }
        QString getGeneratorPrefix() const
        {
            return CGenKey;
        }
        const Sip::Codec &getCodec() const;
        const iRtp::RtpInfraParams &getRtpInfraParams() const
        {
            return m_rtpInfraParams;
        }
        iLogW::LogSettings &getLogSettings()
        {
            return m_logSettings;
        }
        const iLogW::LogSettings &getLogSettings() const 
        {
            return m_logSettings;
        }
    private:
        void RegisterInMetaConfig();
        void ValidateFields();
        QString Help(QString splitter="") const;
    // fields:
    private:
        iLogW::LogSettings m_logSettings;
        const static QString CGenKey;
        Utils::MetaConfig m_cfg;
        Sip::ProfileSip m_sip;
        iRtp::RtpInfraParams m_rtpInfraParams;

        std::string m_codecName; // name of codec

        QString m_writingDefaultFile;
        QString m_playDefaultFile;
        QString m_playDefaultFileWithTimer;
        int m_durationPlayDefaultFile;

        QString m_playAssignedFile;
        QString m_assignedFileName;
    };
} // namespace TestRtpWithSip

#endif

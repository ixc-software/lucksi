#include "stdafx.h"
#include "TestProfile.h"

namespace TestRtpWithSip
{  
    const QString TestProfile::CGenKey("gen");

    TestProfile::TestProfile() :
        m_rtpInfraParams(2, 5073),
        m_codecName("PCMU"),
        m_writingDefaultFile("WritingToDefaultFile"),
        m_playDefaultFile("PlayDefaultFile"),
        m_playDefaultFileWithTimer("PlayDefaultFileWithTimer"),
        m_durationPlayDefaultFile(60000),
        m_playAssignedFile("PlayAssignedFile")
    {
        RegisterInMetaConfig(); 
    }

    //--------------------------------------------------------------------------------------------------

    void TestProfile::RegisterInMetaConfig() 
    {

        m_cfg.Add("//\tSip settings:");
        m_sip.RegisterInMetaConfig(m_cfg);
        m_cfg.Add("\n//\tRtp settings:");        

        m_rtpInfraParams.RegisterInMetaConfig(m_cfg);

        m_cfg.Add(m_codecName, "Codec", "supported only 'PCMA' or 'PCMU'");

        m_cfg.Add("\n//\tTest settings:");        
        m_cfg.Add(m_writingDefaultFile, "Write to file", 
            QString("write RTP stream to the file '") + getDefaultFileName() + "'" );
        m_cfg.Add(m_playDefaultFile, "Play file", 
            QString("play RTP stream from the file '") + getDefaultFileName() + "'" );
        m_cfg.Add(m_playDefaultFileWithTimer, "Play file timeout", 
            QString("play RTP stream from the file '") +  getDefaultFileName() + "' with timeout ...");
        m_cfg.Add(m_durationPlayDefaultFile, "Play duration (ms)");

        m_cfg.Add(m_playAssignedFile, "Play assigned file", "play RTP stream from the assigned file if it exist");
        m_cfg.Add(m_assignedFileName, "Assigned file name");
        
        m_cfg.Add(Help("// "));
        m_cfg.Complete();

    }

    //--------------------------------------------------------------------------------------------------

    void TestProfile::SaveProfile(const std::string &cfgName)
    {
        m_cfg.SaveToFile(QString(cfgName.c_str()));
    }

    //--------------------------------------------------------------------------------------------------        

    void TestProfile::LoadFromLoader(const Utils::MetaConfigLoader& loader) // can throw
    {
        m_cfg.LoadFrom(loader);
        ValidateFields();
        m_sip.InitResipProfile();
        m_rtpInfraParams.SetLocalHostAddress(m_sip.getLocalHost());
    }

    //--------------------------------------------------------------------------------------------------
    
    QString TestProfile::Help(QString splitter) const
    {       
        QString string;
        QTextStream  help(&string);
        help << splitter << "HelpNote: ";
        splitter.prepend("\n");
        help << splitter << "If you call to specific user, you will run associated script to process RTP stream"
            << splitter << "Example 1: if you make call with address sip:[nameWriteToFile]@localhost:" 
            << boost::lexical_cast<std::string>(m_sip.getLocalPort()).c_str() << ","
            << splitter << "\tRTP flow will be written to the file."
            << splitter << "Example 2: if you call to user with the name: gen20,400"
            << splitter << "script will generate signal with amplitude = 20, frequencies = 40";
        return string;
    }

    //--------------------------------------------------------------------------------------------

    std::string TestProfile::ReflectHelp() const
    {        
        std::string help("Current parameters:\n");        
        Utils::StringList sl;
        m_cfg.SaveTo(sl);        
        for (int i = 0; i < sl.size(); ++i)
        {
            help += "\t";
            help += sl.at(i).toStdString();
            help += "/n";
        }
        return Help().toStdString();    
    }

    //--------------------------------------------------------------------------------------------------
    
    void TestProfile::ValidateFields()
    {
        if(!m_rtpInfraParams.IsValid())
            ESS_THROW_MSG(WrongParametrs, 
                "Wrong Rtp port value. Rtp port must be even and max rtp port must be more than min rtp port.");     

        if(m_assignedFileName.isEmpty())
            std::cout << "Warning: filename is empty in config file." << std::endl;
        const Sip::Codec *codec = Sip::Codec::getStaticCodec(m_codecName);
        if (!codec || !(Sip::Codec::ULaw_8000.IsEqual(*codec) || Sip::Codec::ALaw_8000.IsEqual(*codec)))
            ESS_THROW_MSG(WrongParametrs, "Codec mast be PCMU or PCMA");
    }
    //--------------------------------------------------------------------------------------------------

    const Sip::Codec &TestProfile::getCodec() const
    {
        const Sip::Codec *codec = Sip::Codec::getStaticCodec(m_codecName);
        ESS_ASSERT(codec && "Unknown codec");
        return *codec;
    }

} // namespace TestRtpWithSip


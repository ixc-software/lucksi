#include "stdafx.h"
#include "CmdLineProcessing.h"
#include "TestRtpWithSip/TestRtpWithSip.h"
#include "Utils/QtHelpers.h"
#include "Utils/ExeName.h"

namespace
{
    const QString genKey = "-gen";
    const QString helpKey = "-h";
    const QString CDefConfFileName = "default.cfg";

    void ThrowHandeledException(const std::string& msg, const ESS::BaseException& e)
    {
        std::string handeledMsg;
        handeledMsg = msg + " \"" + e.getTextMessage() + "\" in config file ";
        ESS_THROW_MSG(TestRtpWithSip::UserException, handeledMsg);
    }

    void GenDefProfile()
    {
        std::string fileName(Utils::ExeName::GetExeDir());
        fileName += CDefConfFileName.toStdString();
        TestRtpWithSip::TestProfile defaultProfile;
        defaultProfile.SaveProfile(fileName);
        std::cout << "File '" << fileName << "' was generated.\n";
    }

    void HelpMsg()
    {
        std::cout << "Use with:" << std::endl;
        std::cout << "\t" << helpKey << "\t\t- this help" << std::endl;        
        std::cout << "\t" << genKey <<"\t\t- generate default profile " << CDefConfFileName << std::endl;
        std::cout << "\tconfigName\t- start test by profile" << std::endl;        
    }
}

namespace TestRtpWithSip
{       
   
    CmdProcessor::CmdProcessor(const Utils::StringList& arg)
        : m_argList(arg)
    {}

    void CmdProcessor::Run()
    {
        if (m_argList.size() != 2 || m_argList.at(1) == helpKey)
            HelpMsg();
        else if (m_argList.at(1) == genKey)
            GenDefProfile();
        else
            RunByFile();                
    }   

    void CmdProcessor::RunByFile()
    {
        TestProfile profile;
        
        QString arg = m_argList.at(1);
        arg.prepend(Utils::ExeName::GetExeDir().c_str());
        
        try
        {
            // profile.LoadFromLoader( Utils::MetaConfigLoader(arg) );   
            Utils::MetaConfigLoader loader(arg);
            profile.LoadFromLoader( loader );    
        }
        catch(const Utils::MetaConfig::BadConversion& e)
        {            
            ThrowHandeledException("Bad conversion", e);
        }
        catch (Utils::MetaConfig::UnknownFieldName& e) 
        {
            ThrowHandeledException("Unknown Field Name", e);                 
        }
        catch (TestProfile::WrongParametrs& e)
        {
            ThrowHandeledException("Wrong parameters", e);
        }        
        catch (Utils::IStoragable::IoError& e)
        {            
            ESS_THROW_MSG(TestRtpWithSip::UserException, "Can`t open config file.");
        }
        

        TestRtpWithSip::RunTest(profile);
    }

   
} //namespace TestRtpWithSip


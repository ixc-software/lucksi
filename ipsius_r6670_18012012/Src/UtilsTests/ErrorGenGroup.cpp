
#include "stdafx.h"

#include "ErrorGenGroup.h"

#include "Utils/MetaConfig.h"
#include "Utils/Random.h"
#include "Utils/ExeName.h"


namespace 
{
    QString CConfigFileName = "TutErrorGenConfig.ini";

    const QString CEnableTutException = "EnableTutException";
    const QString CFallPercent = "FallPercent";
    const QString CEnableHangUp = "EnableHangUp";
    const QString CEnableEssException = "EnableEssException";
    const QString CEnableEssAssertion = "EnableEssAssertion";

    const int CTimeout = 3000;
    
    const bool CEnableDebugOutput = false;

    // -------------------------------------

    class ConfigReaderClass
    {
        Utils::MetaConfig m_config;

        // registered
        bool m_throwTutException;
        int m_tutExceptionPercent;
        bool m_hangUp;
        bool m_throwEssException;
        bool m_makeEssAssert;
        
        void RegisterMeta()
        {
            m_config.Add(m_throwTutException, CEnableTutException, "test 1");
            m_config.Add(m_tutExceptionPercent, CFallPercent, "test 2");
            m_config.Add(m_hangUp, CEnableHangUp, "test 3");
            m_config.Add(m_makeEssAssert, CEnableEssAssertion, "test 4");
            m_config.Add(m_throwEssException, CEnableEssException, "test 5");
            m_config.Complete();
        }

        void CreateDefaultConfig(const QString &fileName)
        {
            QString config;
            config += "// Set true to enable one failed TUT_ASSERT() per run.\n";
            config += CEnableTutException;
            config += " = false\n\n";

            config += "// Set value from 0 to 100 (note: value > 100 equals to 100).\n"
                      "// <value>% chance to enable one failed TUT_ASSERT() per run.\n";
            config += CFallPercent;
            config += " = 0\n\n";

            config += "// Enable process hang up";
            /*
            config += " (note: timeoute time ";
            config += QString::number(CTimeout);
            config += "ms).";
            */
            config += "\n";
            config += CEnableHangUp;
            config += " = false\n\n";

            config += "// Set true to enable one failed ESS_ASSERT() per run.\n";
            config += CEnableEssAssertion;
            config += " = false\n\n";

            config += "// Set true to throw one ESS exception per run.\n";
            config += CEnableEssException;
            config += " = false\n\n";

            QFile file(fileName);

            if ( !file.open(QIODevice::WriteOnly | QIODevice::Text) )
                TUT_ASSERT(0 && "Default config file creating failed");

            if ( file.write( config.toStdString().c_str() ) < 0 )
                TUT_ASSERT(0 && "Default config file writing failed");
        }

    public:

        ESS_TYPEDEF(ErrorGenException);

        ConfigReaderClass()
        : m_throwTutException(false), m_tutExceptionPercent(0), 
          m_hangUp(false), m_throwEssException(false), m_makeEssAssert(false)
        {
            RegisterMeta();

            QString configFile(Utils::ExeName::GetExeDir().c_str());
            configFile += CConfigFileName;
            if ( !QFile::exists(configFile) ) CreateDefaultConfig(configFile);

            bool wrongConfig = false;
            try
            {
                m_config.LoadFromFile(configFile);
            }
            catch(Utils::MetaConfig::BadConversion &e) { wrongConfig = true; }
            catch(Utils::MetaConfig::UnknownFieldName &e) { wrongConfig = true; }
            catch(Utils::MetaConfigLoader::SyntaxError &e) { wrongConfig = true; }

            TUT_ASSERT(!wrongConfig);
        }

        bool getThrowTutException() { return m_throwTutException; }
        int getTutExceptionPercent() { return m_tutExceptionPercent; }
        bool getHangUp() { return m_hangUp; }
        bool getThrowEssException() { return m_throwEssException; }
        bool getMakeEssAssert() { return m_makeEssAssert; }
    };

	// -------------------------------------------------------

    using namespace TestFw;

	class ErrorGenGroup : public TestGroup<ErrorGenGroup>
	{
		typedef ErrorGenGroup T;

		ConfigReaderClass m_config;

        void MakeTutAssert()
		{
			if ( !m_config.getThrowTutException() ) return;

			TUT_ASSERT(0 && "TutErrorGenException");
		}

		void MakeTutAssertRandomly()
		{
			int errPercent = m_config.getTutExceptionPercent();
			if (errPercent <= 0) return;
	
			int seed = QDateTime::currentDateTime().time().second() 
							+ QDateTime::currentDateTime().time().msec();
			Utils::Random m_random(seed);
	
			int randVal = m_random.Next(100) + 1; // 1-100
	
			if (CEnableDebugOutput)
			{
				std::cout << std::endl << "rand = " << randVal << std::endl
					<< "persent = " << errPercent << std::endl;			
			}
	
			if (randVal <= errPercent) TUT_ASSERT(0 && "TutErrorGenExceptionRand");
		}

		void MakeTimeout()
		{
			if ( m_config.getHangUp() )
			{
				while(true);  // loop forever
			}
		}

		void MakeEssAssert()
		{
			if ( !m_config.getMakeEssAssert() ) return;
			ESS_ASSERT(0 && "ErrorGenAssert");
		}

		void ThrowEssException()
		{
			if ( !m_config.getThrowEssException() ) return;
			ESS_THROW(ConfigReaderClass::ErrorGenException);
		}
		
		ErrorGenGroup(ILauncher &launcher) 
		: TestGroup<ErrorGenGroup>(launcher, "ErrorGen")             
		{
            Add("Make TUT_ASSERT", 					&T::MakeTutAssert);
			Add("Make TUT_ASSERT randomly", 		&T::MakeTutAssertRandomly);
			Add("Make hang up", 					&T::MakeTimeout);
			Add("Make ESS_ASSERT", 					&T::MakeEssAssert);
			Add("Throw exception using ESS_THROW", 	&T::ThrowEssException);
	   }

	public:

		static void AddToLauncher(ILauncher &launcher)
		{
			new ErrorGenGroup(launcher);  // free at launcher
		}
	};

} // namesapce 

// -------------------------------------------------------

namespace UtilsTests
{
    namespace ErrorGen
	{
        void RegisterTestGroup(TestFw::ILauncher &launcher)
		{
			ErrorGenGroup::AddToLauncher(launcher);
		}

	} // namespace ErrorGen 

} // namespace UtilsTests


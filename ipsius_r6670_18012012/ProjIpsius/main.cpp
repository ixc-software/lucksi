
#include "stdafx.h"

#include "BuildInfo.h"

#include "Utils/ExeName.h"
#include "Utils/DelayInit.h"
#include "Utils/StringList.h"
#include "Utils/WeakRef.h"

#include "Utils/CmdLineSplitter.h"
#include "Domain/DomainStartup.h"
#include "Domain/DomainRunner.h"
#include "DRI/FileDRI.h"
#include "DRI/Diff.h"
#include "IpsiusService/AppLauncher.h"

/*
-n Ipsius -ta 50200  -i ../isc/Dss1&Aoz -la 56001 -af Blackfin.isc -at
-n Ipsius -ta 50200  -i ../isc/Pasha -la 56001 -af pult.isc -at
-n Ipsius -ta 50200  -i ../isc/Pasha -la 56001 -af Sip.isc -at

-n Ipsius -ta 50200  -i ../isc/AutoTest -la 56001 -af Main.isc -at
-n Ipsius -ta 50200  -i ../isc/Pult -la 56001 -af Main.isc -at
-n Ipsius -ta 50200  -i ../isc/Ingvar -la 56001 -af Aoz.isc -at

-n Ipsius -i ../isc -t 50200 -l 56001 -v -sc main.isc 
-i ../isc --script MiscAlex\DssRandom.isc --verbose



-n Ipsius -i ../isc/Eugene -ta 50200 -la 56001 -af Blackfin.isc -at

-n Ipsius -ta 50200  -i ../isc/MiscAlex -la 56001 -af pult_alex.isc -at
-n Ipsius -i ../isc/AutoTest -la 56001 -af RegTest.isc -at

-n Ipsius -ta 50200  -i ../isc/Pasha -la 56001 -af ScpTest.isc -at
*/

//-----------------------------------------------------------------------------------

namespace 
{

	class AppExceptionHook : public ESS::ExceptionHook
	{ 
		int m_counter;    

	public:
		AppExceptionHook()
		{}

		void Hook(const ESS::BaseException *pE)  // override
		{
            std::string msg( pE->what() );

			if (dynamic_cast<const ESS::Assertion*>(pE)) 
			{ 
				m_counter++;
			}
			if (dynamic_cast<const ESS::TutException*>(pE))
			{
				m_counter++;
			}
		}
	};


	int RunDomain(const Utils::StringList& cmdLine)
	{
		Domain::DomainExitCode res;

		try
		{   				
			std::string cmpVer = AutoBuildInfo::CmpProtoInfo(); // dummi for forced linking			
			Domain::DomainStartup startup(cmdLine, AutoBuildInfo::FullInfo());
			if (startup.HelpRequested())
			{
				std::cout << startup.Help().toStdString() << std::endl;
				res = Domain::DomainExitErr; 
			}

			Domain::DomainRunner d;
			bool ok = d.Run(startup);
			ESS_ASSERT(ok);

			res = startup.ExitCode();
		}
		catch (Domain::DomainStartup::DomainStartupException &e)
		{
			std::cout << e.getTextMessage() << std::endl;
			res = Domain::DomainExitErr;
		}
		return res;
	}

    void TestC()
    {
        struct DOZ
        {
            int a;
            char b;
            char c;
        };

        DOZ doz[7];

        int *p = &(doz[1].a);
    }

} // namespace 


// ------------------------------------------------------------------------------------

void SaveRefDump()
{
    struct X
    {
        static QByteArray Period()
        {
            QByteArray a;
            QDataStream ds(&a, QIODevice::WriteOnly);
            ds.setByteOrder(QDataStream::LittleEndian);
            
            for(int i = 0; i < 160; ++i)
            {
                ds << (qint16)(i);
            }

            return a;
        }
    };

    QByteArray buff;
    QByteArray a = X::Period();

    for(int i = 0; i < 2000; ++i)
    {
        buff += a;
    }

    QFile f("d:/proj/Ipsius/ProjIpsius/Debug/dump_ref.bin");
    ESS_ASSERT( f.open(QIODevice::WriteOnly | QIODevice::Truncate) );
    f.write(buff);
}

// ------------------------------------------------------------------------------------

void IntCheck()
{
    using namespace Platform;

    ddword i = 0xffffffffffffffffUL;
    int k = i;
    int z = 6;
}


class UniniTest
{
    typedef Platform::byte XBYTE;

    int m_a;
    Platform::dword m_b;
    XBYTE m_c;

public:

    // cppcheck-suppress uninitVar
    UniniTest()
    {
        int b;
        int c = b;
        int d = m_a;
        int e = m_b;
    }
};

// ------------------------------------------------------------------------------------

void CastCheck()
{
    class A { public: virtual ~A() {} };
    class B : public A {};

    B b;
    const A &a = b;

    const B *p  = dynamic_cast<const B*>(&a);
    // B * const p2 = dynamic_cast<B * const>(&a);  // error
}

// ------------------------------------------------------------------------------------

int main(int argc, char *argv[])
{   

    Utils::DelayInitHost::Inst().DoInit();
    Utils::ExeName::Init(argv[0]);        
    AppExceptionHook hook;

    Utils::CmdLineSplitter cls(argc, argv);
	Utils::StringList cmdLine( cls.LeftCount(), cls.LeftArr() );

    if (false)  // debug splitter
    {
        Utils::StringList cmdLineR(cls.RightCount(), cls.RightArr());
    
        std::cout << cmdLine.join("; ").toStdString() << std::endl;
        std::cout << cmdLineR.join("; ").toStdString() << std::endl;
    }

    // UniniTest ut;  // debug  

    // SaveRefDump();
    // ImgRead();
    // Utils::WeakRefTest();
    // IntCheck();
	int res = IpsiusService::AppLauncher::StartApp( cls.RightCount(), cls.RightArr(), 
        boost::bind(RunDomain, boost::cref(cmdLine)) );  

    return res;
}


#include "stdafx.h"

#include "BuildInfo.h"

#include "GUI/udplogviewergui.h"
#include "Core/UlvStartupParams.h"

#include "Utils/ExeName.h"
#include "Utils/DelayInit.h"

namespace
{
    class AppExceptionHook : public ESS::ExceptionHook
    {
        int m_counter;

    public:

        void Hook(const ESS::BaseException *pE)  // override
        {
            int i;

            if (dynamic_cast<const ESS::Assertion*>(pE))
            {
                std::cerr << "Assertion: " << pE->getTextMessage() << std::endl;
                return;
            }

            if (dynamic_cast<const ESS::TutException*>(pE))
            {
                std::cerr << "TutException: " << pE->getTextMessage() << std::endl;
                return;
            }

            std::cerr << "Unknown exception" << pE->what() << std::endl;
        }

    };

    // --------------------------------------------------------------

    void ShowError(const std::string &err)
    {
        /*QErrorMessage e;
        e.setModal(true);
        e.showMessage(QString::fromStdString(err));
        e.exec();*/

        std::cout << err << std::endl;
    }    

} // namespace

// --------------------------------------------------------------

int main(int argc, char *argv[])
{
    // AppExceptionHook hook;

    Utils::DelayInitHost::Inst().DoInit();

    QApplication a(argc, argv);
    Q_INIT_RESOURCE(UdpLogViewerGUI);

    Utils::ExeName::Init(argv[0]);

    Utils::StringList params(argc, argv);
    // parser in DB params
    // Ulv::UdpPackParser packParser;
    boost::scoped_ptr<Ulv::StartupParams> settings;
    try
    {        
        settings.reset(new Ulv::StartupParams);
        settings->Parse(params);
    }
    catch(Ulv::StartupParams::CommandLineError &e)
    {
        ShowError(e.getTextMessage());
        return -1;
    }
    catch(Ulv::StartupParams::HelpRequested &e)
    {
        ShowError(e.getTextMessage());
        return 0;
    }
    catch(Ulv::RecordingErr &e)
    {
        ShowError(e.getTextMessage());
        return -1;
    }

    ESS_ASSERT(settings->IsValid());

    UdpLogViewerGUI w(*settings.get(), AutoBuildInfo::FullInfo());
    w.Show();

    return a.exec();
}



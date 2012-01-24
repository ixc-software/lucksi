#ifndef NOBJARMMODIOTEST_H
#define NOBJARMMODIOTEST_H

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "ModArmIoIntf.h"


#include "iPult/KbGPIO/KbGPIODriver.h"
#include "iPult/PultKeyConst.h"

namespace ModArmIo
{

    class KbGPIOTest : iPult::IKeyboardDriverEvents
    {
        iPult::KbGPIO::KbGPIODriver m_drv;

        // IKeyboardDriverEvents
    private:
        void OnKeyboardDrvChanged(iPult::IKeyboardDriver *pSender, bool isActivated, const QString&)
        {
            ESS_HALT("Unexpected event");
        }
        void OnKeyboardDrvEvent(iPult::IKeyboardDriver *pSender, int keycode, iPult::KeyConst::KeyboardEventType event)
        {            
            std::cout
                    << "KeyCode " << keycode
                    << " Ev " << Utils::EnumResolve(event) << std::endl;
        }
        void OnKeyboardDrvError(iPult::IKeyboardDriver *pSender, const iPult::KeyboardDrvError &err)
        {
            ESS_HALT("Kb error");
        }

//        virtual void OnKeyboardDrvChanged(IKeyboardDriver *pSender, bool isActivated, const QString &extInfo) = 0;
//        virtual void OnKeyboardDrvEvent(IKeyboardDriver *pSender, int keycode, KeyConst::KeyboardEventType event) = 0;
//        virtual void OnKeyboardDrvError(IKeyboardDriver *pSender, const KeyboardDrvError &err) = 0;

    public:
        KbGPIOTest(iCore::MsgThread& thread, iLogW::ILogSessionCreator& log)
            : m_drv(thread, log, *this, iPult::KbGPIO::KbGPIODriverProf())
        {                                  
        }
    };

    //-----------------------------------------------------


    class NObjArmIoTest : public Domain::NamedObject
    {
        Q_OBJECT;

        boost::scoped_ptr<KbGPIOTest> m_kbTest;

        void KbTest(bool start)
        {
            if (start)
                m_kbTest.reset( new KbGPIOTest(getMsgThread(), Log()) );
            else
                m_kbTest.reset();
        }

        bool KbTestActive()
        {
            return m_kbTest != 0;
        }


    // dri:
    public:
        NObjArmIoTest(Domain::IDomain *pDomain, const Domain::ObjectName &name)
            : Domain::NamedObject(pDomain, name)
        {
            //SetGPH9Mode(m_drv, true);
        }



        Q_PROPERTY(bool KbTest WRITE KbTest READ KbTestActive);

        Q_INVOKABLE void SetGPL(bool on, int pin, int pullDown)
        {
            if (pullDown > 3) ThrowRuntimeException("wrong pullDown");         

            // l
            unsigned long pConf = 0x7F008810;
            unsigned long pDat = 0x7f008818;
            unsigned long pPud = 0x7f00881C;

            ModArmIoIntf::CmdList commands;

            int val = on ? 1 : 0;

            int mode = 1; // output
            commands.AddWrite(pConf, mode << pin, 0xf << pin); //shift 0 if gph8, 4 if gph9
            commands.AddWrite(pPud, pullDown << pin, 3 << pin);
            commands.AddWrite(pDat, val << pin, 1 << pin); // led on

            ModArmIoIntf drv;
            drv.RunCommands(commands);
        }

        Q_INVOKABLE void SetGPH8(bool on)
        {

            ModArmIo::ModArmIoIntf::CmdList commands;

            // Set GPH8 configure to Output
            // addr GPHCON1 = 0x7f0080e4 mask = 0x0f val = 1
            commands.AddWrite(0x7f0080e4, 1, 0x0000000f, 20);

            // set gph8 pud 0 (disabled)
            commands.AddWrite(0x7F0080EC, 0, 0x3 << (8 * 2));

            // Set pin to state mute
            // addr GPHDAT = 0x7f0080e8 mask = 1 << 8; val = 1 << 8
            commands.AddWrite(0x7f0080e8, (on ? 0 : 1) << 8, 1 << 8);


            ModArmIo::ModArmIoIntf drv;
            drv.RunCommands(commands);
        }



        ~NObjArmIoTest(){}

    };

} // namespace ModArmIo

#endif // NOBJARMMODIOTEST_H

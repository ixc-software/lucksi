#ifndef _NOBJ_MINI_FLASHER_H_
#define _NOBJ_MINI_FLASHER_H_

#include "stdafx.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "iLog/iLogSessionCreator.h"
#include "MiniFlasher/MfPcProtocol.h"
#include "MfProcessParams.h"

namespace MiniFlasher
{
    class NObjMiniFlasher : 
        public Domain::NamedObject,
        public IMfProcessEvents
	{
		Q_OBJECT;

        typedef NObjMiniFlasher T;

        struct StringBool
		{
            QString Str;
            bool Flag;
			StringBool(const std::string &s, bool f): Str(s.c_str()), Flag(f) {}
            StringBool(const QString &s, bool f): Str(s), Flag(f) {}
		};
        typedef StringBool TaskResult;
        typedef StringBool LogData;

        class IMfProcessRunner : public Utils::IBasicInterface {};

        template<class TProcess>
        class MfProcessRunner : 
            Platform::Thread,
            public IMfProcessRunner
        {
            MfProcessParams m_params;
    
            void run() // override
            {
                TProcess p(m_params);
            }
            
        public:
            MfProcessRunner(MfProcessParams params) : 
              Platform::Thread("MfProcessRunner"), 
              m_params(params) 
            {
                start(); 
            }
        };
        
        boost::scoped_ptr<IMfProcessRunner> m_runner;
        int m_comPort;
		int m_baudRate;
        MfProcessTimeouts m_timeouts;
        boost::shared_ptr<Utils::ThreadSyncEvent> m_abort;

        MfProcessParams MakeParams(iUart::FlowType flowType, const QString &fileName = "",
                                   dword flashOffset = 0, dword flashSize = 0, 
                                   bool runFlashApp = false);
        template<class TProcess>
        void Run(DRI::IAsyncCmd *pAsyncCmd, MfProcessParams params);
        
        void OnFinish(const TaskResult &result);
        void OnLog(const LogData &msg);
        void AbortAsync();

        dword ParamToDword(const QString &param); // can throw
        void CheckPortAndBaudRate();
        
    // IMfProcessEvents impl
    private:
        void MsgLog(const std::string &msg, bool eof = true);
        void MsgProcessEnd(bool ok, const std::string &msg = "");
        boost::shared_ptr<Utils::ThreadSyncEvent> IsAborted();

    /*// Domain::NamedObject 
	private:        
		bool OnPropertyWrite(Domain::NamedObject *pObject, QString propertyName, QString val);*/

    // DRI setters/getters
    private:
        int CommandTimeout() const { return m_timeouts.CommandTimeout; }
        int ConnectTimeout() const { return m_timeouts.ConnectTimeout; }
        int ConnectRepeatInterval() const { return m_timeouts.ConnectRepeatInterval; }

        // TODO: check val > 0 (?)
        void setCommandTimeout(int val) { m_timeouts.CommandTimeout = val; }
        void setConnectTimeout(int val) { m_timeouts.ConnectTimeout = val; }
        void setConnectRepeatInterval(int val) { m_timeouts.ConnectRepeatInterval = val; }
        
	public:
        NObjMiniFlasher(Domain::IDomain *pDomain, const Domain::ObjectName &name);

        Q_PROPERTY(int ComPort READ m_comPort WRITE m_comPort);		
		Q_PROPERTY(int BaudRate READ m_baudRate WRITE m_baudRate);
        // Q_PROPERTY(int CommandTimeout READ CommandTimeout() WRITE setCommandTimeout());
		Q_PROPERTY(int ConnectTimeout READ ConnectTimeout() WRITE setConnectTimeout());
		Q_PROPERTY(int ConnectRepeatInterval READ ConnectRepeatInterval() WRITE setConnectRepeatInterval());

		Q_INVOKABLE void UartBoot(DRI::IAsyncCmd *pAsyncCmd, QString fileName);
		Q_INVOKABLE void LoadImg(DRI::IAsyncCmd *pAsyncCmd, QString fileName, 
                                 QString offset = "0", bool run = false);
        // size == 0 or size > max size --> erase all
        Q_INVOKABLE void EraseFlash(DRI::IAsyncCmd *pAsyncCmd, QString size = "0");
        // size == 0 or size > max size --> read all
        Q_INVOKABLE void FlashDump(DRI::IAsyncCmd *pAsyncCmd, QString destFileName, 
                                   QString size, QString offset = "0");
        Q_INVOKABLE void FlashDumpVerify(DRI::IAsyncCmd *pAsyncCmd, QString file, 
                                         QString offset = "0");
        Q_INVOKABLE void FlashTest(DRI::IAsyncCmd *pAsyncCmd);
        Q_INVOKABLE void UartBenchmark(DRI::IAsyncCmd *pAsyncCmd, int timeToRun);
	};
    
} // namespace MiniFlasher 

#endif



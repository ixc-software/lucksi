 #include "stdafx.h"
#include <QDataStream>

#include "Utils/SafeRef.h"
#include "Utils/CRC32.h"

#include "iCore/MsgTimer.h"
#include "iCore/SyncTimer.h"
#include "iCoreTests/iCoreLogTest.h"
#include "iCore/ThreadRunner.h"

#include "iNet/SocketError.h"
#include "iNet/SocketData.h"
#include "iNet/IUdpSocketToUser.h"
#include "iNet/MsgUdpSocket.h"
#include "iNetTests/DataFlowInf.h"


namespace
{
    //--------------------------------------------------------------------------------------
    // параметры теста 

    class TestParams
    {
        static const int CCountSocket = 6;
        static const int PayloadSize = 40;
        static const int CStartInterval = 100;        
        static const int CSendInterval = 5;        
        static const int CFinishInterval = 200;        
        static const int CRepeatCount = 50;        

        iCoreTests::ILoggable &m_log;
        const Utils::HostInf m_localHost;

        iNetTests::DataFlowInf m_testResult;                

    public:

        TestParams(iCoreTests::ILoggable &log, const Utils::HostInf &host) :
            m_log(log), 
            m_localHost(host){}
        
        iCoreTests::ILoggable &getLog() {   return m_log;   }

        //----------------------------------------------------------            

        void SaveTestResult(const iNetTests::DataFlowInf &data)
        {
            m_testResult.Add(data);
        }

        //----------------------------------------------------------            

        void LogResult()    {   getLog().Log(m_testResult.Str());   }
        
        //----------------------------------------------------------            

        bool TestIsOk() const   {   return m_testResult.IsOk(); }

        //----------------------------------------------------------            
        
        int getCountSocket() const    {   return CCountSocket;    }
        
        //----------------------------------------------------------            
        
        const Utils::HostInf &LocalHostInf() const  {   return m_localHost;  }

        //----------------------------------------------------------                    

        std::string getAddress() const
        {
            return LocalHostInf().Address();
        }

        //----------------------------------------------------------            

        int getPort() const
        {
            return LocalHostInf().Port();
        }
       
        //----------------------------------------------------------            
        
        int getMaxTestDuration() const 
        {
            return 0xFFFF;
        }
        
        //----------------------------------------------------------            
        
        int getPayloadSize() const  {    return PayloadSize; }

        //----------------------------------------------------------            

        int getStartInterval() const    {   return CStartInterval;  }
        
        //----------------------------------------------------------            
        
        int getSendInterval() const {   return CSendInterval;   }
        
        //----------------------------------------------------------            
        
        int getFinishInterval() const   {   return CFinishInterval; }
        
        //----------------------------------------------------------                        
        
        int getRepeatCount() const {    return CRepeatCount;    }

    };
    
    
    //--------------------------------------------------------------------------------------

    class UserUdpSocket;

    class IOwnerOfUserUdpSocket : public Utils::IBasicInterface 
    {
    public:
        virtual void Finish(UserUdpSocket *finishedIf) = 0;
    };

    //--------------------------------------------------------------------------------------

	class UserUdpSocket :
		public iCore::MsgObject,
		public iNet::IUdpSocketToUser
	{
		typedef UserUdpSocket T;
	public:
		
		UserUdpSocket(iCore::MsgThread &thread, TestParams &params,
                      IOwnerOfUserUdpSocket &owner,
					  int localPort, int remoutePort) :
			iCore::MsgObject(thread), m_params(params),
            m_owner(owner), m_timer(this, &T::OnSend), 
            m_socket(new iNet::MsgUdpSocket(thread, this, Utils::HostInf(m_params.getAddress(), localPort))),
            m_remouteHost(m_params.getAddress(), remoutePort)
        {
            m_timer.Start(getSendInterval(), true);
        }
          
        //-----------------------------------------------------------
        
        const iNetTests::DataFlowInf &dataFlowInf() const
		{
			return m_data;
		}
        
        //-----------------------------------------------------------

        iNetTests::DataFlowInf &dataFlowInf()
        {
            return m_data;
        }

    //-----------------------------------------------------------
	private:
        
        static const int IsAnswerPack = 0;
        static const int IsDataPack = 1;

        //----------------------------------------------------------

    // интерфейс ITcpSocketToUser
	private:

        void ReceiveData(boost::shared_ptr<iNet::SocketData> data)
        {
			ESS_ASSERT(m_socket->IsEqual(data->ID()));            
            
            QDataStream in(&data->getData(), QIODevice::ReadOnly);            
            int size;
            in >> size;

            int command;
            in >> command;

            switch (command) 
            {
                case IsAnswerPack:
                {
                    int confirmedNumber;
                    in >> confirmedNumber;				
                    
                    // получили подтверждение передачи нашего пакета
                    dataFlowInf().ReceiveAnswerPack(confirmedNumber);
                }
                break;
                case IsDataPack: 
                {
                    int receiveNumber;				
                    in >> receiveNumber;				
                    dataFlowInf().ReceiveDataPack(receiveNumber);
                    
                    QByteArray answerPack;
                    {
                        QDataStream out(&answerPack, QIODevice::WriteOnly);
                        out << IsAnswerPack << dataFlowInf().getReceiveNumber();
                    }

                    Send(answerPack);
                }
                break;
                default:
                    TUT_ASSERT(0 && "Unknown packet");				
            }
        }
        
        //-----------------------------------------------------------
		
        void SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error)
		{
			// логируем сообщение об ошибке
			// возможно анализируем и продолжаем/завершаем работу
			ESS_ASSERT(m_socket->IsEqual(error->ID()));
		}

		// end of: интерфейс ITcpSocketToUser
	private:
       
        //-----------------------------------------------------------

        int getSendInterval() const     {   return m_params.getSendInterval();  }

        //-----------------------------------------------------------

        int getFinishInterval() const   {   return m_params.getFinishInterval();    }
                
        //-----------------------------------------------------------		

        void Send(QByteArray data)
        {
            QByteArray dataBlock;
            {
                QDataStream out(&dataBlock, QIODevice::WriteOnly);
                out << data.size();
            }
            dataBlock += data;

            boost::shared_ptr<iNet::SocketData> ptrData(new iNet::SocketData(m_remouteHost, dataBlock));
            m_socket->SendData(ptrData);
        }
        
        //-----------------------------------------------------------                
        
        void OnSend(iCore::MsgTimer *pT)
		{
            if (m_finishedFlag.Get()) return;            

            QByteArray data(m_params.getPayloadSize(), '*');
            QDataStream out(&data, QIODevice::WriteOnly);
            out << IsDataPack << dataFlowInf().incSendNumber();
            Send(data);
            
            if (dataFlowInf().getSendNumber() < m_params.getRepeatCount())
                return;
            m_finishedFlag.Set(true);
            pT->Stop();
            pT->BindEvent(this, &T::OnFinish);
            pT->Start(getFinishInterval());

		}
        
        //-----------------------------------------------------------

        void OnFinish(iCore::MsgTimer *pT)
		{
            m_params.SaveTestResult(dataFlowInf());
            m_owner.Finish(this);
		}

    private:
        TestParams &m_params;
        IOwnerOfUserUdpSocket &m_owner;
        iCore::MsgTimer m_timer;
        Utils::AtomicBool m_finishedFlag;
           
        boost::shared_ptr<iNet::MsgUdpSocket> m_socket;
        Utils::HostInf m_remouteHost;
        iNetTests::DataFlowInf m_data;
	};

    //--------------------------------------------------------------------------------------

    class UdpSocketUsersList 
	{
    public:
        //----------------------------------------------------------		
        
        int getCurrentSize() const {    return m_sockets.size();    }
        
        //----------------------------------------------------------
		
        void Add(UserUdpSocket *userOfSocket)   {   m_sockets.push_back(userOfSocket);  }

        //----------------------------------------------------------
		
        void Delete(UserUdpSocket *userOfSocket)
		{
            List::iterator i = std::find(m_sockets.begin(), m_sockets.end(), userOfSocket);
           
            ESS_ASSERT(i != m_sockets.end());

            m_sockets.erase(i);
		}
       
	private:
        typedef std::list<UserUdpSocket*> List;
        List m_sockets;
	};
		
    //--------------------------------------------------------------------------------------
   
    Utils::AtomicInt GCountTestUdp;

    class TestUdp : public iCore::MsgObject, public IOwnerOfUserUdpSocket 
    {
        typedef TestUdp T;
    public:

        TestUdp(iCore::IThreadRunner &runner, TestParams &params) :
            iCore::MsgObject(runner.getThread()),
            m_done(runner.getCompletedFlag()),
            m_countSocket(params.getCountSocket())
        {
            ESS_ASSERT(runner.getThread().InCurrentThreadContext());
            GCountTestUdp.Inc();
            Run(params);
        }
        
        //----------------------------------------------------------            

        ~TestUdp()
        {
            GCountTestUdp.Dec();
        }
        
        //----------------------------------------------------------            
       
    private:

        void Run(TestParams &params)
        {
            for(int i = 0; i < params.getCountSocket(); ++i)
            {
                int localPort   = params.getPort() + i;
                int remoutePort = (i&1) ? localPort -1 :  localPort + 1;
                UserUdpSocket *testSocketUser = new UserUdpSocket(getMsgThread(), params, *this, localPort, remoutePort);
                m_sockets.Add(testSocketUser);
            }
        }
        
        //----------------------------------------------------------        
        
        void Finish(UserUdpSocket *userOfSocket)
        {
            m_sockets.Delete(userOfSocket);
            PutMsg(this, &T::OnDeleteUser, userOfSocket);
            
            if(--m_countSocket)
                return;
            // тест завершен, все сокеты удалены
            // сообщение нужно отправить для того чтобы обеспечить вызов OnDeleteUser  
            PutMsg(this, &T::OnFinish);
        }

        //----------------------------------------------------------

        void OnDeleteUser(UserUdpSocket *userOfSocket)
        {
            // сообщение используется для вызова деструктора userOfSocket
            // по выходу из функции вызовется деструктор userOfSocket
        }

        //----------------------------------------------------------        

        void OnFinish()
        {
            AsyncSetReadyForDeleteFlag(m_done);
        }
        
        //----------------------------------------------------------        
    
    private:        
        Utils::AtomicBool &m_done;        
        int m_countSocket;        
        UdpSocketUsersList m_sockets;
    };
};

//--------------------------------------------------------------------------------------


namespace iNetTests
{
    void UdpTest(bool silenceMode)
    {
        iCoreTests::Logger log(silenceMode);

        TestParams params(log, Utils::HostInf("127.0.0.1", 3050));
        iCore::ThreadRunner test(Platform::Thread::LowPriority, params.getMaxTestDuration());

        if (!test.Run<TestUdp>("TestUdp", params))
        {
            TUT_ASSERT(0 && "Timeout");
        }
        else
        {
            TUT_ASSERT(!GCountTestUdp.Get());
            params.LogResult();
            TUT_ASSERT(params.TestIsOk() && "Error in data flow"); 
            log.Log("Udp test: Ok!\n");
        }
    }
};


	

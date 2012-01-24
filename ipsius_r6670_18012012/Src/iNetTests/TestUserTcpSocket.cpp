#include "stdafx.h"	
#include "iNet/MsgTcpSocket.h"
#include "iNet/SocketError.h"
#include "iNet/SocketData.h"

#include "ProfileTcpTest.h"
#include "TestTcpPack.h"
#include "TestUserTcpSocket.h"	

namespace iNetTests
{

    TestUserTcpSocket::TestUserTcpSocket(iCore::MsgThread &thread,
        ProfileTcpTest &profile,
        IFinish &owner,
        boost::shared_ptr<iNet::ITcpSocket> serverSocket) :
	    iCore::MsgObject(thread), 
        m_timer(this, &T::OnSend),                 
	    m_owner(owner),             
        m_profile(profile),
	    m_number(-1), 
        m_isClient(false),
        m_socket(serverSocket)
    {
        // конструктор серверной стороны                 
        m_socket->LinkUserToSocket(this);
    }

    //-----------------------------------------------------------

    TestUserTcpSocket::TestUserTcpSocket(iCore::MsgThread &thread,
        ProfileTcpTest &profile,
        IFinish &owner,
        int number) :
		iCore::MsgObject(thread), 
        m_timer(this, &T::OnSend), 
        m_owner(owner), 
        m_profile(profile),
		m_number(number), 
        m_isClient(true),
        m_socket(new iNet::MsgTcpSocket(thread, this))
    {}
    
    TestUserTcpSocket::~TestUserTcpSocket()
    {

    }

    //-----------------------------------------------------------
        
    void TestUserTcpSocket::Start(const Utils::HostInf &host)
	{
		TUT_ASSERT(isClient() && "Error: call start() on server socket");				

		m_hostInf = host;
        m_socket->ConnectToHost(m_hostInf);
	}


    // интерфейс ITcpSocketToUser
    //-----------------------------------------------------------

	void TestUserTcpSocket::Connected(iNet::SocketId id)
	{
		ESS_ASSERT(m_socket->IsEqual(id));
		
		// только для клиентского сокета
		TUT_ASSERT(isClient() && "Error: call Connected() on server socket");				
		m_timer.Start(m_profile.getSendInterval(), true);
	}
        
    //-----------------------------------------------------------

    void TestUserTcpSocket::ReceiveData(boost::shared_ptr<iNet::SocketData> data)
	{
		ESS_ASSERT(m_socket->IsEqual(data->ID()));

        m_packet += data->getData();
        TestTcpPack pack;
        while (pack.Parse(m_packet))
        {
		    if (numberIsPresent() && getNumber() != pack.getSocketNumber())
                ESS_ASSERT(0 && "Receive msg from other number");
            
            if(pack.isAnswer())
            {
                ESS_ASSERT(numberIsPresent() && "Receive answer to unnumbered user");
                dataFlowInf().ReceiveAnswerPack(pack.getNumber());
            }
            else
            {
                if(!isClient())
                {
//                    pT->Start(m_profile.getSendInterval(), true);
                    setNumber(pack.getSocketNumber());
                }
                dataFlowInf().ReceiveDataPack(pack.getNumber());
                TestTcpPack answer(getNumber(), dataFlowInf().getReceiveNumber());
                m_socket->SendData(answer.SendAnswer());
            }
        }
	}

    //-----------------------------------------------------------

	void TestUserTcpSocket::Disconnected(boost::shared_ptr<iNet::SocketError> error)
	{
    	ESS_ASSERT(m_socket->IsEqual(error->ID()));	
        m_timer.Stop();            

        m_profile.SaveTestResult(dataFlowInf());
        m_owner.Finish(this);
	}
        
    //-----------------------------------------------------------
	
    void TestUserTcpSocket::SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error)
	{
		// логируем сообщение об ошибке
		// возможно анализируем и продолжаем/завершаем работу
		ESS_ASSERT(m_socket->IsEqual(error->ID()));
	}

    //-----------------------------------------------------------                

    void TestUserTcpSocket::OnSend(iCore::MsgTimer *pT)
    {
        if (dataFlowInf().getSendNumber() < m_profile.getRepeatCount())
        {
            TestTcpPack pack(getNumber(), dataFlowInf().incSendNumber());
            return m_socket->
                SendData(pack.SendData(m_profile.getPayloadSize()));
        }
        if(!dataFlowInf().IsFinished())
            return;

        pT->Stop();
        pT->BindEvent(this, &T::OnFinish);
        pT->Start(m_profile.getFinishInterval());
    }

    //-----------------------------------------------------------		

    void TestUserTcpSocket::OnFinish(iCore::MsgTimer *pT)
    {
        m_socket->DisconnectFromHost();            
    }
    
    //-----------------------------------------------------------

    bool TestUserTcpSocket::isClient() const 
	{
		return m_isClient;
	}
        
    //-----------------------------------------------------------

    int TestUserTcpSocket::getNumber() const
    {
        return m_number;
    }
    
    //-----------------------------------------------------------
    
    void TestUserTcpSocket::setNumber(int number)
    {
        m_number = number;
    }

    //-----------------------------------------------------------

    bool TestUserTcpSocket::numberIsPresent() const
    {
        return m_number != -1;
    }

    //-----------------------------------------------------------

    const iNetTests::DataFlowInf &TestUserTcpSocket::dataFlowInf() const
    {
        return m_data;
    }

    //-----------------------------------------------------------

    iNetTests::DataFlowInf &TestUserTcpSocket::dataFlowInf()
    {
        return m_data;
    }

};



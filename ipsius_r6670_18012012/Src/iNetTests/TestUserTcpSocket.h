#ifndef _TEST_USER_TCP_SOCKET_H_
#define _TEST_USER_TCP_SOCKET_H_

#include "stdafx.h"

#include "iCore/MsgThread.h"
#include "iCore/MsgObject.h"
#include "iCore/MsgTimer.h"

#include "iNet/ITcpSocketToUser.h"
#include "iNet/ITcpSocket.h"
#include "IFinish.h"


namespace iNetTests
{
    class ProfileTcpTest;

    class TestUserTcpSocket :
		    public iCore::MsgObject,
            public IFinish::Item,
		    public iNet::ITcpSocketToUser
    {
        typedef TestUserTcpSocket T;
	public:
		
        // server side
        TestUserTcpSocket(iCore::MsgThread &thread,
            ProfileTcpTest &profile,
            IFinish &owner,
            boost::shared_ptr<iNet::ITcpSocket> serverSocket);

        // client side
		TestUserTcpSocket(iCore::MsgThread &thread,
            ProfileTcpTest &profile,
            IFinish &owner,
            int number);
        
        ~TestUserTcpSocket();
        
        void Start(const Utils::HostInf &host);

    // интерфейс ITcpSocketToUser
	private:
		void Connected(iNet::SocketId id);
        void ReceiveData(boost::shared_ptr<iNet::SocketData> data);
		void Disconnected(boost::shared_ptr<iNet::SocketError> error);
        void SocketErrorOccur(boost::shared_ptr<iNet::SocketError> error);

    // methods
    private:       
        void OnSend(iCore::MsgTimer *pT);
        void OnFinish(iCore::MsgTimer *pT);

        bool isClient() const; 
        int getNumber() const;
        void setNumber(int number);
        bool numberIsPresent() const;

        const iNetTests::DataFlowInf &dataFlowInf() const;
        iNetTests::DataFlowInf &dataFlowInf();
    //members
    private:
        iCore::MsgTimer m_timer;
        IFinish &m_owner;        
        ProfileTcpTest &m_profile;
        iNetTests::DataFlowInf m_data;
        int m_number;// порядковый номер обьекта
        bool m_isClient;
        boost::shared_ptr<iNet::ITcpSocket> m_socket;
        Utils::HostInf m_hostInf;
        QByteArray m_packet;
	};

};

#endif

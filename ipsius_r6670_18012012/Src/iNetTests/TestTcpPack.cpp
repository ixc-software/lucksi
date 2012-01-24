#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "TestTcpPack.h"

namespace 
{
    static const int IsAnswerPack = 0;
    static const int IsDataPack = 1;
    static const int EndDataPack = -1;

    enum 
    {
        USER_CONTENT = 0xEC
    };

};

namespace iNetTests
{
    TestTcpPack::TestTcpPack(int socketNumber, int packNumber) :
        m_size(sizeof(TestTcpPack)),
        m_socketNumber(socketNumber),
        m_number(packNumber),
        m_command(EndDataPack),
        m_endFlag(EndDataPack)
        {}

    //------------------------------------------------------------

    TestTcpPack::TestTcpPack() :
        m_size(EndDataPack),
        m_socketNumber(EndDataPack),
		m_number(0),		
        m_command(EndDataPack),
        m_endFlag(EndDataPack)
        {}
    
    //------------------------------------------------------------

    QByteArray TestTcpPack::SendData(int sizeData)
    {
        QByteArray data(sizeof(TestTcpPack) + sizeData, 0);
        QDataStream out(&data, QIODevice::WriteOnly);          
        m_command = IsDataPack;
        out <<  m_size + sizeData << m_socketNumber << m_number << m_command;
        for(int i = 0; i < sizeData; ++i) out << qint8(USER_CONTENT); 
        out << EndDataPack;
        return data;
    }

    //------------------------------------------------------------

    QByteArray TestTcpPack::SendAnswer()
    {
        QByteArray answerPack(sizeof(TestTcpPack), 0);
        QDataStream out(&answerPack, QIODevice::WriteOnly);          
        m_command = IsAnswerPack;
        out <<  m_size << m_socketNumber << m_number << m_command << EndDataPack;
        return answerPack;
    }

    //------------------------------------------------------------

    bool TestTcpPack::Parse(QByteArray &data)
    {
        QByteArray temp(data);
        
        int i = 0;
        {
            QByteArray end;
            QDataStream out(&end, QIODevice::WriteOnly);
            out << EndDataPack;
            i = data.indexOf(end, 0);
        }
        if(i == -1)
            return false;

        if (sizeof(TestTcpPack) > data.size())
        {
            TUT_ASSERT(0 && "Packet mast be more long.");				            
            return false;
        }
        QDataStream in(&data, QIODevice::ReadOnly);            
        Parse(in);

        if(getSize() > data.size())
        {
            TUT_ASSERT(0 && "Packet mast be more long.");				            
            return false;
        }
        data.remove(0, i + sizeof(EndDataPack));
        return true;
    }

    //------------------------------------------------------------

    void TestTcpPack::Parse(QDataStream &in)
    {
        in >> m_size;
        in >> m_socketNumber;
        in >> m_number;
        in >> m_command;
        TUT_ASSERT((m_command == IsAnswerPack || 
            m_command == IsDataPack) && "Unknown packet");				
    }

    //------------------------------------------------------------

    bool TestTcpPack::isAnswer() const 
    {
        return m_command == IsAnswerPack;
    }

    //------------------------------------------------------------

    int TestTcpPack::getSize() const
    {
        return m_size;
    }

    //------------------------------------------------------------

    int TestTcpPack::getSocketNumber() const 
    {
        return m_socketNumber;
    }

    //------------------------------------------------------------

    int TestTcpPack::getNumber() const 
    {
        return m_number;
    }
};

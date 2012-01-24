#ifndef _TEST_TCP_DATA_PACK_H_
#define _TEST_TCP_DATA_PACK_H_

#include "stdafx.h"

namespace iNetTests
{
    class TestTcpPack 
    {
    public:
        TestTcpPack(int socketNumber, int packNumber);
        TestTcpPack();
        QByteArray SendData(int sizeData);
        QByteArray SendAnswer();
        bool Parse(QByteArray &data);

        bool isAnswer() const;
        int getSize() const;
        int getSocketNumber() const;
        int getNumber() const; 
    private:
        void Parse(QDataStream &in);
    //members
    private:
        int m_size;
        int m_socketNumber;
        int m_number;
        int m_command;
        int m_endFlag;
    };
}
#endif

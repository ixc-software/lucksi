#ifndef __DATA_FLOW_INF_H__
#define __DATA_FLOW_INF_H__

namespace iNetTests
{
    class DataFlowInf 
    {
        // класс содержит информацию о порядковых номерах переданных/полученых пакетов
        // и проверяет последовательность

        int m_sendNumber;
        int m_sendNumberConfirmed;
        int m_receiveNumber;

        int m_errorDataNumber;
        int m_errorAnswerNumber;

    public:            
        DataFlowInf();

        void Add(const DataFlowInf &src);

        void ReceiveAnswerPack(int confirmedNumber);

        void ReceiveDataPack(int receiveNumber);

        int incSendNumber();

        int getSendNumber() const; 

        int getSendNumberConfirmed() const;

        int getReceiveNumber() const;

        int getErrorDataNumber() const; 

        int getErrorAnswerNumber() const; 

        bool IsFinished() const 
        {
            return getSendNumber() == getSendNumberConfirmed();
        }

        bool IsOk() const 
        {
            return IsFinished() && !getErrorDataNumber() && !getErrorAnswerNumber();
        }

        std::string Str();

    }; // end of DataFlowInf 
}

#endif

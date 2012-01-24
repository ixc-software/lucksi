#include "stdafx.h"
#include "DataFlowInf.h"

namespace iNetTests
{

    DataFlowInf::DataFlowInf() :  
        m_sendNumber(0), m_sendNumberConfirmed(0), m_receiveNumber(0), 
        m_errorDataNumber(0), m_errorAnswerNumber(0)
    {}

    void DataFlowInf::Add(const DataFlowInf &src)
    {
        m_sendNumber += src.m_sendNumber;
        m_sendNumberConfirmed += src.m_sendNumberConfirmed;
        m_receiveNumber += src.m_receiveNumber;

        m_errorDataNumber += src.m_errorDataNumber;
        m_errorAnswerNumber += src.m_errorAnswerNumber;
    }
        
    //----------------------------------------------------------            

    void DataFlowInf::ReceiveAnswerPack(int confirmedNumber)
    {
      // получили подтверждение передачи нашего пакета
      if (confirmedNumber == m_sendNumberConfirmed +1 || 
          confirmedNumber <= m_sendNumber)
          m_sendNumberConfirmed++;
      else
          m_errorAnswerNumber++;
    }

    //----------------------------------------------------------            

    void DataFlowInf::ReceiveDataPack(int receiveNumber)
    {
      if(receiveNumber != m_receiveNumber +1)
          m_errorDataNumber++;                
      m_receiveNumber++; 	
    }

    //----------------------------------------------------------            

    int DataFlowInf::incSendNumber() 
    {
      return ++m_sendNumber;
    }

    //----------------------------------------------------------            

    int DataFlowInf::getSendNumber() const 
    {
      return m_sendNumber;
    }

    //----------------------------------------------------------                        

    int DataFlowInf::getSendNumberConfirmed() const 
    {
      return m_sendNumberConfirmed;
    }
    
    //----------------------------------------------------------            

    int DataFlowInf::getReceiveNumber() const 
    {
      return m_receiveNumber;
    }

    //----------------------------------------------------------            

    int DataFlowInf::getErrorDataNumber() const 
    {
      return m_errorDataNumber;
    }

    //----------------------------------------------------------            

    int DataFlowInf::getErrorAnswerNumber() const 
    {
      return m_errorAnswerNumber;
    }

    std::string DataFlowInf::Str()
    {
        std::stringstream s;
        
        s << " Send packet: " << getSendNumber() << std::endl;
        s << " Confirmed send packet: " << getSendNumberConfirmed() << std::endl;
        s << " Receive packet: " << getReceiveNumber() << std::endl;
        s << " Error Data Number: " << getErrorDataNumber() << std::endl;
        s << " Error Answer Number: " << getErrorAnswerNumber();
        
        return s.str();
    }

}



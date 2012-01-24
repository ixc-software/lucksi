
#include "stdafx.h"
#include "DataExchangeTestState.h"
#include "Utils/Random.h"
#include "time.h"

namespace IsdnTest
{
    using Platform::byte;

    namespace TestStateImpl
    {
        enum
        {
            CUseTrueRandom = true
        };


        namespace Aux
        {
            // Тестирующий 3й уровень для подтеста обмена сообщениями
            class DataExchangeSide : public L3State
            {

            public:

                DataExchangeSide(LapdTest& fsm, IL3Test* pL3, ISDN::IsdnL2::Layer2* l2) : 
                  L3State(pL3), m_fsm(fsm), m_pl2(l2), m_seed(1)
                  {
                      m_state = SEND_I;
                      ResetState();
                  }

                  bool Process()
                  {
                      if(m_state == SEND_I) return DoSendI();
                      if(m_state == RECV_I) return m_completed;

                      if(m_state == SEND_U) return DoSendU();
                      if(m_state == RECV_U) return m_completed;

                      TUT_ASSERT(0 && "Invalid State in DataExchangeSide");
                      return false;
                  }

                  void EnterState_SendI(bool dropMode = false)
                  {
                      m_random.setSeed( CUseTrueRandom ? time(0) : 35654);
                      m_state = SEND_I;
                      GenerateParam(dropMode);
                      ResetState();                      
                  }

                  void EnterState_RecvI()
                  {
                      ResetState();
                      ResetParam();
                      m_state = RECV_I;   
                  }

                  void EnterState_SendU()
                  {
                      m_state = SEND_U;
                      GenerateParam();                      
                      ResetState();                      
                  }

                  void EnterState_RecvU()
                  {
                      ResetState();            
                      ResetParam();
                      m_state = RECV_U;   
                  }

            private:

                void ResetState()
                {
                    m_random.setSeed(m_seed); 
                    m_sended = 0;
                    m_recvied = 0;      
                    m_completed = false;
                    m_synchro = false;
                }

                void ResetParam()
                {
                    m_sendSessionCount = 0;
                    m_PacketSize = 0;
                    m_seed = 1;
                }

                void GenerateParam(bool dropMode = false)
                {
                    //m_sendSessionCount = 1 + m_random.Next( m_pl2->GetMaxWindow() - 1 );
                    switch(m_state)
                    {
                    case(SEND_I):                        
                        m_sendSessionCount = dropMode 
                            ? 1 + m_random.Next(m_fsm.GetProfile().m_MaxIPacketsInDropMode)
                            : 1 + m_random.Next(m_fsm.GetProfile().m_MaxIPackets);
                        break;
                    case(SEND_U):
                        m_sendSessionCount = 1 + m_random.Next(m_fsm.GetProfile().m_MaxUPackets);
                        break;
                    default:
                        TUT_ASSERT(0 && "GenerateParam in wrong state");
                    }                     
                    m_PacketSize = m_random.Next();
                    m_seed = m_random.Next();
                }

                
                QVector<byte> GetRandomPacket()
                {
                    // сгенерить случайный пакет размером m_PacketSize с помощью m_random
                    QVector<byte> data;
                    for(int i = 0; i < m_PacketSize; ++i)
                        data.push_back( m_random.NextByte() );
                    return data;
                }

                               
                QVector<byte> GetSynchroPacket()
                {
                    QVector<byte> data;
                    TUT_ASSERT(m_sendSessionCount/* && "Number packet by session = 0"*/);
                    data.push_back( m_sendSessionCount );
                    data.push_back( m_PacketSize );
                    data.push_back( m_seed );
                    return data;
                }
                
                void SetParam(QVector<byte> l2data)
                {
                    TUT_ASSERT(l2data.size() == 3);
                    m_sendSessionCount = l2data.at(0);
                    m_PacketSize = l2data.at(1);
                    m_seed = l2data.at(2);
                    m_random.setSeed(m_seed);
                    m_synchro = true;
                }
                

                bool DoSendI()
                {                    
                    if(!m_synchro) 
                    {                        
                        GetL3()->GetIntfDown()->DataReq( GetSynchroPacket() );
                        m_synchro = true;
                        return false;
                    }

                    int sendCount = 1 + m_randomAsync.Next(m_sendSessionCount);

                    while(m_sended < m_sendSessionCount)
                    {
                        if (sendCount == 0) return false;                        
                        GetL3()->GetIntfDown()->DataReq( GetRandomPacket() );
                        m_sended++;

                        sendCount--;
                    }

                    return true;                    
                }

                bool DoSendU()
                {                    
                    if(!m_synchro) 
                    {                        
                        GetL3()->GetIntfDown()->UDataReq( GetSynchroPacket() );
                        m_synchro = true;
                        return false;
                    }
                    if (m_sended >= m_sendSessionCount) return true;                    
                    GetL3()->GetIntfDown()->UDataReq( GetRandomPacket() );
                    m_sended++;
                    return false;
                }

                
                void DataInd(QVector<byte> l2data)  // override
                {
                    TUT_ASSERT(m_state == RECV_I);
                    
                    if(!m_synchro) 
                    {
                        SetParam(l2data);                        
                        return;
                    }

                    TUT_ASSERT(m_synchro || m_sendSessionCount || m_PacketSize || m_seed!=1);

                    if (l2data.size() != m_PacketSize) 
                    {                           
                        TUT_ASSERT(0 && "Error1 in Exchange test");
                    }

                    for(int i = 0; i < m_PacketSize; ++i) 
                        if (l2data.at(i) != m_random.NextByte())
                        {                            
                            TUT_ASSERT(0 && "Error2 in Exchange test");
                        }
                        
                        m_recvied++;

                        if (m_recvied == m_sendSessionCount) m_completed = true;
                }

                
                void UDataInd(QVector<byte> l2data)  // override
                {
                    TUT_ASSERT(m_state == RECV_U);

                    if(!m_synchro) 
                    {
                        SetParam(l2data);                        
                        return;
                    }
                    
                    TUT_ASSERT(m_synchro || m_sendSessionCount || m_PacketSize || m_seed);

                    if (l2data.size() != m_PacketSize) 
                    {                           
                        TUT_ASSERT(0 && "Error3 in Exchange test");
                    }

                    for(int i = 0; i < m_PacketSize; ++i) 
                        if (l2data.at(i) != m_random.NextByte())
                        {                            
                            TUT_ASSERT(0 && "Error4 in Exchange test");
                        }
                        
                        m_recvied++;

                        if (m_recvied == m_sendSessionCount) m_completed = true;
                }

                void EstablishInd() {} // override
                void EstablishConf() {} // override

                void ReleaseConf() {} // override
                void ReleaseInd() {/*m_pl2->owner->EstablishReq();*/} // override

                virtual void OnEmptyEvent(){} // override

                enum InnerState
                {
                    SEND_I,
                    RECV_I,
                    SEND_U,
                    RECV_U,
                };

				InnerState m_state;
                LapdTest& m_fsm;
                int m_sended;
                int m_recvied;
                Utils::Random m_random;
                Utils::Random m_randomAsync;
                bool m_completed;
                byte m_PacketSize;        // число байт в пакете, сделать случайным
                byte m_sendSessionCount;   // число пакетов, отправляемых за сессию (сделать случайным, не больше размера окна)
                bool m_synchro;
                byte m_seed;
                ISDN::IsdnL2::Layer2* m_pl2;

            };
        } // namespace Aux

        //---------------------------------------------------------------------


        class DataExchangeTestState: public TestState
        {

            typedef Aux::DataExchangeSide L3;

        public:

            DataExchangeTestState(LapdTest& fsm, int maxDuration, TestState* pNextState): 
              TestState(fsm, "DataExchangeTestState" , maxDuration),
                  m_totalCycles(fsm.GetProfile().m_DataExchangeCycle), m_currCycle(0),
                  m_left(*new L3(fsm, &fsm.GetL3Left(), fsm.GetL2Left().getL2() ) ), 
                  m_right(*new L3(fsm, &fsm.GetL3Right(), fsm.GetL2Right().getL2() ) ),
                  //m_left (fsm, fsm.GetL3Left().GetL3Test(), fsm.GetL2Left().GetImpl() ) , 
                  //m_right (fsm, fsm.GetL3Right().GetL3Test(), fsm.GetL2Right().GetImpl() ) ,
                  m_completed(false),
                  m_pNextState(pNextState)
              {
                  GetOwner().GetL3Left().SwitchTo(&m_left); 
                  GetOwner().GetL3Right().SwitchTo(&m_right); 

                  EnterState(SEND_I_FROM_LEFT_SAFE); // установка начального состояния
              }

              ~DataExchangeTestState()
              { 
                  //delete &m_left;
                  //delete &m_right;
                  
              }

              void Do()
              {
                  Proc();
                  bool ok = Completed();
                  if (!ok ) return;

                  SwitchState( m_pNextState );
              }

        private:

            enum InnerState
            {
                SEND_I_FROM_LEFT_SAFE,
                SEND_I_FROM_RIGHT_SAFE,
                SEND_U_FROM_LEFT_SAFE,
                SEND_U_FROM_RIGHT_SAFE,
                SEND_I_FROM_LEFT_BAD,
                SEND_I_FROM_RIGHT_BAD,
                TEST_COMPLETED,
            };

            void Proc()
            {
                if (Completed()) return;

                bool leftRes = m_left.Process();
                bool rightRes = m_right.Process();

                if (leftRes && rightRes)
                {
                    InnerState next = NextState(m_state);
                    if (next == TEST_COMPLETED) 
                    {
                        ResetDrv();
                        m_completed = true;
                        return;
                    }   
                    EnterState(next);
                }
            }

            void EnterState(InnerState state)  // переход в состояние
            {
                m_state = state;
                std::string msg;

                ResetDrv();

                if (m_state == SEND_I_FROM_LEFT_SAFE)
                {
                    msg = "DataExchangeTestState: go to SEND_I_FROM_LEFT_SAFE";
                    GetOwner().GetLogger()->Log(msg);
                    m_left.EnterState_SendI();
                    m_right.EnterState_RecvI();
                    return;
                }

                if (m_state == SEND_I_FROM_RIGHT_SAFE)
                {
                    msg = "DataExchangeTestState: go to SEND_I_FROM_RIGHT_SAFE";
                    GetOwner().GetLogger()->Log(msg);
                    m_right.EnterState_SendI();
                    m_left.EnterState_RecvI();
                    return;
                }

                if (m_state == SEND_U_FROM_LEFT_SAFE)
                {
                    msg = "DataExchangeTestState: go to  SEND_U_FROM_LEFT_SAFE";
                    GetOwner().GetLogger()->Log(msg);
                    m_left.EnterState_SendU();
                    m_right.EnterState_RecvU();
                    return;
                }

                if (m_state == SEND_U_FROM_RIGHT_SAFE)
                {
                    msg = "DataExchangeTestState: go to SEND_U_FROM_RIGHT_SAFE";
                    GetOwner().GetLogger()->Log(msg);
                    m_right.EnterState_SendU();
                    m_left.EnterState_RecvU();
                    return;
                }

                if (m_state == SEND_I_FROM_LEFT_BAD)
                {    
                    msg = "DataExchangeTestState: go to SEND_I_FROM_LEFT_BAD";
                    GetOwner().GetLogger()->Log(msg);
                    GetOwner().GetDrvLeft().PacketKillOn();
                    GetOwner().GetDrvRight().PacketKillOn();
                    m_left.EnterState_SendI(true);
                    m_right.EnterState_RecvI();
                    return;
                }

                if (m_state == SEND_I_FROM_RIGHT_BAD)
                {
                    msg = "DataExchangeTestState: go to SEND_I_FROM_RIGHT_BAD";
                    GetOwner().GetLogger()->Log(msg);
                    GetOwner().GetDrvLeft().PacketKillOn();
                    GetOwner().GetDrvRight().PacketKillOn();
                    m_right.EnterState_SendI(true);
                    m_left.EnterState_RecvI();
                    return;
                }

                TUT_ASSERT(0 && "Unknown state");
            }

            InnerState NextState(InnerState state)
            {
                InnerState endState = TEST_COMPLETED;
                if (state == endState - 1)  
                {
                    m_currCycle++;
                    if (m_currCycle >= m_totalCycles) return TEST_COMPLETED;
                    return SEND_I_FROM_LEFT_SAFE; // first state
                }

                return InnerState(state + 1);
            }

            bool Completed() { return m_completed; }

            void ResetDrv()
            {
                GetOwner().GetDrvLeft().PacketKillOff();
                GetOwner().GetDrvRight().PacketKillOff();                
            }

            InnerState m_state;
            int m_currCycle;
            int m_totalCycles;
            L3 &m_left, &m_right;
            bool m_completed;
            TestState* m_pNextState;
        };

        TestState *GetDataExchangeTest(LapdTest& fsm, int maxDuration, TestState* pNextState)
        {
            return new DataExchangeTestState(fsm, maxDuration, pNextState);
        }

    } // namespace DataExchange

} // namespace IsdnTest

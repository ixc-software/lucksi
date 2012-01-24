#ifndef __AOZFREQRECVPOOL__
#define __AOZFREQRECVPOOL__

#include "Utils/ManagedList.h"
#include "Utils/BidirBuffer.h"
#include "TdmMng/TdmException.h"
#include "TdmMng/IFreqRecvOwner.h"
#include "DrvAoz/AozShared.h"
#include "iCmp/ChMngProto.h"  //   #include "iCmp/ChMngProtoShared.h"  

#include "itdmfreqrecv.h"
#include "FreqRecvDtmf.h"

namespace FreqRecv
{


    /*  оболочка для полиморфного приемника частоты
        класс трансформирует режим работы приемника в управление им 

        испльзуется два вариенат -- удаленный и локальный (с обработкой данных)
     */

    template<class TRecvIntf>
    class FreqRecvT : boost::noncopyable
    {
        std::string m_name;
        iCmp::FreqRecvMode m_mode;
        bool m_lineBusy;
        bool m_dialBegin;

    protected:

        boost::scoped_ptr<TRecvIntf> m_recv;

    private:

        void SetMode(iCmp::FreqRecvMode mode)
        {
            ESS_ASSERT(m_recv != 0);

            iCmp::FreqRecvMode prevMode = m_mode;
            m_mode = mode;
            
            if (mode == iCmp::FrOn)
            {
                m_recv->On(); 
                return;
            }

            if (mode == iCmp::FrOff)
            {
                m_recv->Off(); 
                return;
            }

            if (mode == iCmp::FrAutoOn)
            {
                m_recv->Off();  // trigger on only on next line state change
                return;
            }

            if (mode == iCmp::FrAutoOnOff)
            {
                LineState(m_lineBusy);
                return;
            }

            // error
            {
                using namespace TdmMng;

                m_mode = prevMode;  // restore

                std::ostringstream oss;
                oss << "Bad mode " << mode;
                ESS_THROW_T(TdmException, TdmErrorInfo(terFreqRecvError, oss.str()));
            }

        }

    public:

        FreqRecvT(const std::string &name) : 
          m_name(name),
          m_mode(iCmp::FrOff),
          m_lineBusy(false),
          m_dialBegin(false)
        {
        }

        void Reset()
        {
            if (m_recv != 0) m_recv->Off();
        }

        const std::string& Name() const { return m_name; }

        template<class TOwner>
        bool Set(TOwner &owner, iCmp::FreqRecvMode mode, const std::string &params)
        {
            if ((m_recv == 0) || ((m_recv != 0) && !m_recv->Equal(params)))
            {
                m_recv.reset( owner.CreateFreqRecv(m_name, params) );
            }

            if (m_recv == 0)
            {
                using namespace TdmMng;
                std::string msg = "Bad FreqRecv name " + m_name;
                ESS_THROW_T(TdmException, TdmErrorInfo(terFreqRecvError, msg));
            }

            SetMode(mode);

            return true;
        }

        void LineState(bool busy)
        {
            // set dial begin
            if (m_recv != 0)
            {
                if ( (!m_lineBusy && busy) && 
                     (m_name == iCmp::PcCmdSetFreqRecvMode::CRecvDTMF()) )
                {
                    m_dialBegin = true;
                }
            }

            // set new line state
            m_lineBusy = busy;

            if (m_recv == 0) return;

            if (m_mode == iCmp::FrAutoOn)
            {
                if (busy) m_recv->On();
            }

            if (m_mode == iCmp::FrAutoOnOff)
            {
                if (busy) m_recv->On();
                     else m_recv->Off();
            }
        }

        bool PeekDialBegin()
        {
            if (!m_dialBegin) return false;

            m_dialBegin = false;
            return true;
        }

    };

    // ----------------------------------------

    // удаленный вариант
    class FreqRecvRemote : public FreqRecvT<ITdmFreqRecvRemote>
    {

    public:

        FreqRecvRemote(const std::string &name) : FreqRecvT<ITdmFreqRecvRemote>(name)
        {
        }

    };

    // ----------------------------------------

    // локальный вариант 
    class FreqRecvLocal : public FreqRecvT<ITdmFreqRecvLocal>
    {

    public:

        FreqRecvLocal(const std::string &name) : FreqRecvT<ITdmFreqRecvLocal>(name)
        {
        }

        void ProcessData(const std::vector<Platform::int16> &data)
        {
            if (m_recv == 0) return;
            if (!m_recv->WaitData()) return;

            // process
            m_recv->ProcessData(data);
        }

        std::string PeekEvent()
        {
            return (m_recv == 0) ? "" : m_recv->PeekEvent();
        }

        bool NeedDataProcess()
        {
            if (m_recv == 0) return false;
            return m_recv->WaitData();
        }

    };

    // --------------------------------------------------------
    
    /* набор всех приемников частоты, привязанных к одному каналу
       каждый приемник частоты имеет уникальное имя, фактически определяющее его тип

       два варианта класса -- для локальной работы (на уровне TDM) и
       удаленной (на уровне Sfx, при этом нет передачи данных) 
    */

    template<class TFreqRecv, class TOwner>
    class FreqRecvPoolT : boost::noncopyable
    {

    protected:

        TOwner &m_owner;
        Utils::ManagedList<TFreqRecv> m_list;

    private:

        TFreqRecv* FindByName(const std::string &name)
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i]->Name() == name) return m_list[i];
            }

            return 0;
        }

        void ResetAll()
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                m_list[i]->Reset();
            }
        }

        void ProcessLineState(bool busy)
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                m_list[i]->LineState(busy);
            }
        }

    public:

        FreqRecvPoolT(TOwner &owner) : m_owner(owner)
        {
        }

        bool Command(const std::string &recvName, iCmp::FreqRecvMode mode, const std::string &params)
        {
            TFreqRecv *pRecv = FindByName(recvName);

            if (pRecv == 0)
            {
                pRecv = new TFreqRecv(recvName);
                m_list.Add(pRecv);
            }

            return pRecv->Set(m_owner, mode, params);
        }

        void RouteAozEvent(TdmMng::AozLineEvent e)
        {
            // board state
            if ( (e == TdmMng::aleBoardOn) || (e == TdmMng::aleBoardOff) )
            {
                ResetAll(); 
            }

            // line state
            if (e == TdmMng::aleLineBusy) ProcessLineState(true);
            if (e == TdmMng::aleLineFree) ProcessLineState(false);

        }

        void ProcessEvent(TFreqRecv &recv, 
            const std::string &devName, int chNum, const std::string &data)
        {
            // dial begin
            if ( recv.PeekDialBegin() )
            {
                m_owner.PushDialBeginEvent(devName, chNum);
            }

            // data event
            m_owner.PushFreqRecvEvent(devName, chNum, recv.Name(), data);
        }

        void ProcessEvent(const std::string &recvName, 
            const std::string &devName, int chNum, const std::string &data)
        {
            TFreqRecv *pRecv = FindByName(recvName);
            if (pRecv == 0) return;  // throw - ?

            ProcessEvent(*pRecv, devName, chNum, data);
        }
               
    };

    // ----------------------------------------

    // remote version -- typdef for FreqRecvPoolT<FreqRecvRemote, InnerSfxOwner>
    /*
    class FreqRecvPoolRemote : public FreqRecvPoolT<FreqRecvRemote>
    {
    public:
    }; */

    // ----------------------------------------

    typedef FreqRecvPoolT<FreqRecvLocal, TdmMng::IFreqRecvOwner> BaseForLocal;

    // add data process to remote version 
    class FreqRecvPoolLocal : public BaseForLocal
    {
        boost::scoped_ptr<Utils::BidirBuffer> m_rawData;  // compressed data
        std::vector<Platform::int16> m_lineData;          // data in line code

        bool NeedDataProcess()
        {
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i]->NeedDataProcess()) return true;
            }

            return false;
        }

    public:

        FreqRecvPoolLocal(TdmMng::IFreqRecvOwner &owner) : BaseForLocal(owner)
        {
        }

        // with events callback
        void ProcessData(const Utils::BidirBuffer &buff, bool forced,
            const std::string &devName, int chNum)
        {
            if (!forced)
            {
                if (!NeedDataProcess()) return;
            }

            // reserve data in m_dataBuff
            if (m_lineData.size() != buff.Size())
            {
                ESS_ASSERT( m_lineData.empty() );
                m_lineData.resize( buff.Size() );
            }

            // decode
            m_owner.GetChCodec().DecodeBlock(buff.Front(), &m_lineData[0], m_lineData.size());

            // process
            for(int i = 0; i < m_list.Size(); ++i)
            {
                FreqRecvLocal *p = m_list[i];

                // process data
                p->ProcessData(m_lineData);

                // peek all events
                while(true)
                {
                    std::string e = p->PeekEvent();
                    if (e.empty()) break;

                    ProcessEvent(*p, devName, chNum, e);
                    // m_owner.PushFreqRecvEvent(p->Name(), e);
                }

            }
        }

        Utils::BidirBuffer* PeekDataBuffer()
        {
            if (!NeedDataProcess()) return 0;

            if (m_rawData == 0)  // lazy init
            {
                m_rawData.reset( m_owner.CreateBidirBuffer() );
            }

            m_rawData->Clear();
            return m_rawData.get();
        }

        static ITdmFreqRecvLocal* CreateLocalFreqRecv(const std::string &name, 
                                                      const std::string &params)
        {
            if (name == iCmp::PcCmdSetFreqRecvMode::CRecvDTMF())
            {
                FreqRecv::ITdmFreqRecvLocal *p = new RecvDtmfLocal(params);
                return p;
            } 

            return 0;            
        }


    };
    
    
}  // namespace FreqRecv

#endif

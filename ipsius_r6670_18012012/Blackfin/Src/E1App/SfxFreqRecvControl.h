#ifndef __SFXFREQRECVCONTROL__
#define __SFXFREQRECVCONTROL__

namespace E1App
{

    class ISfxFreqRecvProto : public Utils::IBasicInterface
    {
    public:

        virtual void CmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, bool enable) = 0;

    };

    // ---------------------------------------------------

    class SfxFreqRecvControl : boost::noncopyable
    {

        class ISfxFreqRecvOwner : public Utils::IBasicInterface
        {
        public:

            virtual FreqRecv::ITdmFreqRecvRemote* CreateFreqRecv(const std::string &name, 
                const std::string &params) = 0;

            virtual void PushDialBeginEvent(const std::string &devName, int chNum) = 0;
            virtual void PushFreqRecvEvent(const std::string &devName, int chNum,
                const std::string &freqRecvName, const std::string &data) = 0;

        };

        typedef FreqRecv::FreqRecvPoolT<FreqRecv::FreqRecvRemote, ISfxFreqRecvOwner> FreqRecvPoolRemote;

        class Channel : public FreqRecv::ITdmFreqRecvRemote
        {
            ISfxFreqRecvProto &m_proto; 
            const int m_chNum;

            bool m_active;

        // FreqRecv::ITdmFreqRecvRemote impl
        private:

            bool Equal(const std::string &params)
            {
                return (params == "");
            }

            void On()
            {
                if (m_active) return;

                const std::string dtmf = iCmp::PcCmdSetFreqRecvMode::CRecvDTMF();
                m_proto.CmdSetFreqRecvMode(m_chNum, dtmf, "", true);

                m_active = true;
            }

            void Off()
            {
                if (!m_active) return;

                const std::string dtmf = iCmp::PcCmdSetFreqRecvMode::CRecvDTMF();
                m_proto.CmdSetFreqRecvMode(m_chNum, dtmf, "", false);

                m_active = false;
            }

        public:

            Channel(ISfxFreqRecvProto &proto, int chNum) : 
              m_proto(proto),
              m_chNum(chNum),
              m_active(false)
            {
            }

        };

        class CreateBind : public ISfxFreqRecvOwner
        {
            ISfxFreqRecvProto &m_proto; 
            const int m_chNum;
            TdmMng::EventsQueue &m_queue;

        // ISfxFreqRecvOwner impl
        private:

            FreqRecv::ITdmFreqRecvRemote* CreateFreqRecv(const std::string &name, 
                const std::string &params)
            {
                if (name == iCmp::PcCmdSetFreqRecvMode::CRecvDTMF())
                {
                    return new Channel(m_proto, m_chNum);
                }

                return 0;
            }

            void PushDialBeginEvent(const std::string &devName, int chNum)
            {
                typedef FreqRecv::FreqRecvHelper Helper;

                Helper::PushDialBeginEvent(m_queue, devName, chNum);
            }

            void PushFreqRecvEvent(const std::string &devName, int chNum,
                const std::string &freqRecvName, const std::string &data)
            {
                typedef FreqRecv::FreqRecvHelper Helper;

                Helper::PushFreqRecvEvent(m_queue, devName, chNum, freqRecvName, data);
            }

        public:

            CreateBind(ISfxFreqRecvProto &proto, int chNum, TdmMng::EventsQueue &q) : 
              m_proto(proto),
              m_chNum(chNum),
              m_queue(q)
            {
            }

        };

        ISfxFreqRecvProto &m_proto;
        const Platform::dword m_voiceChMask;

        Utils::ManagedList<ISfxFreqRecvOwner> m_chCreateBinders;
        Utils::ManagedList<FreqRecvPoolRemote> m_list;

        static void ThrowBadChNumber(int chNum)
        {
            using namespace TdmMng;
            std::string msg = "Bad ch number " + Utils::IntToString(chNum);
            ESS_THROW_T(TdmException, TdmErrorInfo(terFreqRecvError, msg));
        }

        void ThrowOnBadChannel(int chNum)
        {
            if ( (chNum >= m_list.Size()) || (m_list[chNum] == 0) ) ThrowBadChNumber(chNum);
        }

    public:

        SfxFreqRecvControl(ISfxFreqRecvProto &proto, 
                           Platform::dword voiceChMask,
                           TdmMng::EventsQueue &q) : 
          m_proto(proto),
          m_voiceChMask(voiceChMask),
          m_list(true, 32, true)
        {
            const int CMaxChannels = sizeof(voiceChMask) * 8;

            for(int i = 0; i < CMaxChannels; ++i)
            {
                if (voiceChMask & (1 << i))
                {
                    ISfxFreqRecvOwner *pOwner = new CreateBind(proto, i, q);
                    m_chCreateBinders.Add(pOwner);
                    m_list.Add( new FreqRecvPoolRemote(*pOwner) ); 
                }
                else
                {
                    m_list.Add(0);
                }
            }
        }

        void CmdSetFreqRecvMode(int chNum, const std::string &recvName, 
            const std::string &params, iCmp::FreqRecvMode mode)
        {
            ThrowOnBadChannel(chNum);

            m_list[chNum]->Command(recvName, mode, params);
        }

        void RouteAozEvent(int chNum, TdmMng::AozLineEvent e)
        {
            bool toAny = (chNum < 0);
            
            for(int i = 0; i < m_list.Size(); ++i)
            {
                if (m_list[i] == 0) continue;

                if (toAny || (chNum == i))
                {
                    m_list[i]->RouteAozEvent(e);
                }
            }

        }

        void PushFreqRecvEvent(const std::string &devName, int chNum,
            const std::string &freqRecvName, const std::string &data)
        {
            ThrowOnBadChannel(chNum);

            m_list[chNum]->ProcessEvent(freqRecvName, devName, chNum, data);
        }

    };
    
    
    
}  // namespace E1App

#endif

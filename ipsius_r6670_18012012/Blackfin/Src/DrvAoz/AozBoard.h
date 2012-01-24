#ifndef __AOZBOARD__
#define __AOZBOARD__

#include "Platform/Platform.h"

#include "iLog/LogWrapper.h"
#include "TdmMng/TdmHAL.h"
#include "TdmMng/EventsQueue.h"
#include "iCore/MsgObject.h"
#include "iCmp/ChMngProto.h"
#include "Utils/ManagedList.h"
#include "Utils/StatisticElement.h"
#include "BfDev/BfTimerCounter.h"
#include "DevIpTdm/AozInit.h"
#include "DevIpTdm/BfLed.h"

#include "AozShared.h"
#include "AozChannel.h"
#include "AozTdmOutput.h"

namespace DrvAoz
{
    using boost::scoped_ptr;
    using iCmp::AbCofidecParams;
    using Platform::word;
    using iCmp::BfAbEvent;
    using iCmp::ChMngProtoParams;

    enum
    {
        CBoardStateToColorLed       = true,
        CTraceSignByteBuffSize      = 0,        // set non-zero for debug
    };

    // ---------------------------------------------------------------

    struct AozBoardStats
    {
        // все интервалы в тиках таймера BfTimerCounter - !
        int TdmBlockCollisions;
        Utils::StatElementForInt FullTdmWrite;
        Utils::StatElementForInt FullTdmRead;

        AozBoardStats()
        {
            Clear();
        }

        void Clear()
        {
            TdmBlockCollisions = 0;
            FullTdmWrite.Clear();
            FullTdmRead.Clear();
        }

        std::string ToString()
        {
            std::ostringstream oss;

            ESS_UNIMPLEMENTED;

            return oss.str();
        }
    };

    // ---------------------------------------------------------------
    
    class AozBoard : 
        public iCore::MsgObject,
        public TdmMng::ITdmOwner,
        IChannelOwner
    {

        enum 
        {
            CTdmFrameSize = AozTdmOutput::CTdmOutputFrameSize,
        };

        BOOST_STATIC_ASSERT(CTdmFrameSize == 16);

        // состояние платы (доступна/отсутствует) через проверку ее сигнатуры 
        class BoardState
        {
            enum
            {
                CCountToBoardLost    = 3,
                CCountToBoardAvaible = 5,
            };

            bool m_boardAvaible;
            const byte m_signature;

            int m_switchCounter;  // 0 -- disabled, when countdown to 0, mode changed

        public:

            BoardState(byte signature) : 
              m_boardAvaible(false), 
              m_signature(signature),
              m_switchCounter(0)
            {
            }

            bool Avaible() const { return m_boardAvaible; }

            // return true, if state changed
            bool Update(byte sign)
            {
                bool signOk = (sign == m_signature);
                bool switchTry = (signOk && !m_boardAvaible) || (!signOk && m_boardAvaible);

                if (switchTry)
                {
                    // first try
                    if (m_switchCounter <= 0)
                    {
                        m_switchCounter = m_boardAvaible ? CCountToBoardLost : CCountToBoardAvaible;
                        return false;
                    }

                    --m_switchCounter;
                    if (m_switchCounter > 0) return false;

                    // switch state
                    m_boardAvaible = !m_boardAvaible;
                    return true;
                }

                m_switchCounter = 0;
                return false;
            }

        };

        AozBoardStats m_stats;
        BfDev::BfTimerCounter m_timer;

        scoped_ptr<iLogW::LogSession> m_logSession;
        iLogW::LogRecordTag m_tagInfo;

        const iCmp::BfInitDataAoz m_setup;
        TdmMng::TdmHAL &m_hal;

        TdmMng::EventsQueue m_events;

        Utils::ManagedList<AozChannel> m_channels;
        AozTdmOutput m_output;
        word m_tdmInFrame[CTdmFrameSize];  // incoming frame snapshot

        BoardState m_state;

        // debug
        std::vector<Platform::byte> m_signTraceBuff;
        int m_totalSignCheck, m_singOkCount;

        enum
        {
            CSignTS = 0,
            CPulseLo = 1,
            CPulseHi = 9,
            CHookLo = 2,
            CHookHi = 10,
        };

        static bool ChNumberIsCorrect(int chNumber, bool hiPartTdm)
        {
            if (!hiPartTdm)
            {
                if (chNumber >= 0 && chNumber <= 15) return true;
            }
            else
            {
                if (chNumber >= 16 && chNumber <= 31) return true;
            }

            return false;
        }

        void VerifySignature(byte sign)
        {
            if (!m_state.Update(sign)) return;

            // state to led
            if (CBoardStateToColorLed)
            {
                using DevIpTdm::BfLed;
                BfLed::SetColor( m_state.Avaible() ? DevIpTdm::GREEN : DevIpTdm::RED );	
            }

            // notify channels about board state change
            for(int i = 0; i < m_channels.Size(); ++i)
            {
                m_channels[i]->Enable( m_state.Avaible() );
            }

            // send event
            std::string params = ChMngProtoParams::ToParams( m_state.Avaible() );
            PushChannelEvent( AozEvent(0xffff, BfAbEvent::CBoardState(), params) );
        }

        void SendSignTrace()
        {
            std::ostringstream oss;
            oss << "Sign " << m_singOkCount << "/" << m_totalSignCheck << ": "
                << Utils::DumpToHexString(m_signTraceBuff);

            PushChannelEvent( AozEvent(0xffff, "Trace", oss.str()) );
            
            m_signTraceBuff.clear();
        }

        static bool GetStateBit(const word *p, int number, int indexLo, int indexHi)
        {
            ESS_ASSERT(indexLo < 16);
            ESS_ASSERT(indexHi < 16);
            ESS_ASSERT(number < 16);

            int index = indexLo;
            if (number >= 8)
            {
                number -= 8;
                index = indexHi;
            }

            return (p[index] & (1 << number)) ? true : false;
        }

        void ProccessFrameForChannelsPulse(const word *pFrame)
        {
            for(int i = 0; i < m_channels.Size(); ++i)
            {
                bool hook  = GetStateBit(pFrame, m_channels[i]->Number(), CHookLo,  CHookHi);
                bool pulse = GetStateBit(pFrame, m_channels[i]->Number(), CPulseLo, CPulseHi);
                m_channels[i]->ProcessPulse(hook, pulse);
            }
        }

        void CreateChannels()
        {
            ESS_ASSERT(m_channels.Size() == 0);

            TdmMng::TdmChannelsSet chSet;
            if (!chSet.Parse(m_setup.VoiceChannels))
            {
                ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadVoiceChannelString); 
            }

            int max = chSet.MaxValue();

            for(int i = 0; i <= max; ++i)
            {
                if (!chSet.IsExist(i)) continue;

                if (!ChNumberIsCorrect(i, m_setup.HiTdmChPart))
                {
                    ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadVoiceChannelString); 
                }

                int chNum = !m_setup.HiTdmChPart ? i : i - 16;

                // add
                m_channels.Add( new AozChannel(*this, m_setup.DefaultLineSetup, chNum, 
                    m_setup.PulseCapturePeriodMs) );
            }
        }

    // IChannelOwner impl
    private:

        void PushChannelEvent(const AozEvent &event)
        {
            m_events.Push(AbAozEvent(m_setup.DeviceName, event) );
        }

        AozTdmOutput& TdmOutput()
        {
            return m_output;
        }

        bool DebugTraceEnabled()
        {
            return m_logSession->LogActive();
        }

        void ChannelLog(const std::string &s)
        {
            if (!m_logSession->LogActive()) return;

            *m_logSession << m_tagInfo << s << iLogW::EndRecord;
        }


    // TdmMng::ITdmOwner impl
    private:

        void TdmFrameRead(TdmMng::TdmReadWrapper &block) 
        {
            m_timer.Reset();  // begin

            int offset = m_setup.HiTdmChPart ? 16 : 0;
            byte sign = block.GetFrame(0, offset)[CSignTS];

            // update snapshot
            std::memcpy(&m_tdmInFrame, block.GetFrame(0, offset), sizeof(m_tdmInFrame));

            // debug
            if (CTraceSignByteBuffSize > 0)
            {
                bool signOk = (sign == 0xa6);
                ++m_totalSignCheck;
                if (signOk) ++m_singOkCount;

                using DevIpTdm::BfLed;
                BfLed::SetColor( signOk ? DevIpTdm::GREEN : DevIpTdm::RED );	

                m_signTraceBuff.push_back(sign);
                if (m_signTraceBuff.size() >= CTraceSignByteBuffSize) SendSignTrace();
            }

            // signature verify
            VerifySignature(sign);

            // process PULSE and HOOK for channels
            if (m_state.Avaible())
            {
                int step = 8 * m_setup.PulseCapturePeriodMs;
                int count = block.BlockSize() / step;  // store this - ?
                ESS_ASSERT(count * step == block.BlockSize());

                for(int i = 0; i < count; ++i)
                {
                    const word *pFrame = block.GetFrame(i * step, offset);
                    ProccessFrameForChannelsPulse(pFrame);
                }

            }

            // fix time
            m_stats.FullTdmRead.Add( m_timer.GetCounter() );
        }

        void TdmFrameWrite(TdmMng::TdmWriteWrapper &block) 
        {
            m_timer.Reset();  // begin

            m_output.UpdateRings();

            int offset = m_setup.HiTdmChPart ? 16 : 0;

            for(int i = 0; i < block.BlockSize(); ++i)
            {
                word *pOutput = block.GetFrame(i, offset);
                m_output.WriteOutput(pOutput);
            }

            // fix time
            m_stats.FullTdmWrite.Add( m_timer.GetCounter() );
        }

    public:

        AozBoard(iCore::MsgThread &thread, 
            Utils::SafeRef<iLogW::ILogSessionCreator> logCreator,
            const iCmp::BfInitDataAoz &data,
            TdmMng::TdmHAL &hal) : 
            iCore::MsgObject(thread),
            m_timer(BfDev::BfTimerCounter::GetFreeTimer()),
            m_logSession(logCreator->CreateSession(data.DeviceName + "_AOZ", true)),
            m_tagInfo(m_logSession->RegisterRecordKindStr("Info")),
            m_setup(data),
            m_hal(hal),
            m_events("AozBoard"),
            m_output(data.DefaultCofidecSetup, data.DmaIrqPeriodMs(data.SignSportNum), 
                     data.RingMaxDelayMs, data.RingSearchPoints, data.RingMaxActiveCount),
            m_state(data.BoardStateSignByte),
            m_totalSignCheck(0), 
            m_singOkCount(0)
        {
            ESS_ASSERT( DevIpTdm::AozInit::InitDone() );

            // fill input
            for(int i = 0; i < CTdmFrameSize; ++i) m_tdmInFrame[i] = i;

            // verify pulse poll period
            {
                int capacity = (m_setup.SignSportNum == 0) ? 
                    m_setup.Sport0BlockCapacity :
                    m_setup.Sport1BlockCapacity;
                capacity /= 8;
                if (capacity % m_setup.PulseCapturePeriodMs != 0) 
                {
                    ESS_THROW_T(TdmMng::TdmException, TdmMng::terBadPulseCapturePeriodMs); 
                }
            }

            // create channels
            CreateChannels();

            // register in HAL
            m_hal.RegisterTdm(data.SignSportNum, data.SignSportChNum, 
                *this, data.UseAlaw /* dummi */);

            // log
            ChannelLog("AOZ created!");
        }

        ~AozBoard()
        {
            m_hal.UnregTdm(*this);
        }

        void GetTdmSnapshot(std::vector<byte> &tdmIn, 
                            std::vector<byte> &tdmOut)
        {
            // input
            {
                tdmIn.clear();
                tdmIn.reserve(CTdmFrameSize);

                for(int i = 0; i < CTdmFrameSize; ++i)
                {
                    tdmIn.push_back( m_tdmInFrame[i] );
                }

            }

            // output
            {
                Platform::word buff[CTdmFrameSize];
                m_output.WriteOutput(buff);

                tdmOut.clear();
                tdmOut.reserve(CTdmFrameSize);

                for(int i = 0; i < CTdmFrameSize; ++i)
                {
                    tdmOut.push_back( buff[i] );
                }
            }
        }


        const std::string Name() const
        {
            return m_setup.DeviceName;
        }

        void BlockProcess(TdmMng::ITdmBlock &block)
        {
            // read
            block.ProcessReadFor(*this, 0);

            // write
            block.ProcessWriteFor(*this);
        }

        void BlockCollision()
        {
            ++m_stats.TdmBlockCollisions;
        }

        void SetCofidecSetup(int chMask, const iCmp::AbCofidecParams &params)
        {
            // verify mask - only all channels - ?
            // ...

            m_output.SetCofidec(params);
        }

        void SetLineParams(int chMask, const iCmp::AbLineParams &params)
        {
            for(int i = 0; i < m_channels.Size(); ++i)
            {
                m_channels[i]->SetLineParams(chMask, params);
            }
        }

        void Command(int chMask, const std::string &cmdName, const std::string &cmdParams)
        {
            for(int i = 0; i < m_channels.Size(); ++i)
            {
                m_channels[i]->Command(chMask, cmdName, cmdParams);
            }
        }

        void SendAllEventsFromQueue(SBProto::ISafeBiProtoForSendPack &proto,
                                    TdmMng::IIEventsQueueItemDispatch &hook)
        {
            m_events.SendAll(proto, &hook);
        }

        bool Avaible() const 
        { 
            return m_state.Avaible(); 
        }

     };
    
}  // namespace DrvAoz


#endif

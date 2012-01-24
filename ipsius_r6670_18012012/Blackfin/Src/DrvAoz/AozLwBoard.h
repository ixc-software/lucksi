#ifndef __AOZLWBOARD__
#define __AOZLWBOARD__

#include "DevIpTdm/BfLed.h"

namespace DrvAoz
{
    
    class AozLwBoard : 
        public iCore::MsgObject,
        public TdmMng::ITdmOwner
    {

        enum 
        {
            CTdmFrameSize = 16,
        };

        BOOST_STATIC_ASSERT(CTdmFrameSize == 16);


        const iCmp::BfInitDataAoz m_setup;
        TdmMng::TdmHAL &m_hal;

        int m_counter;

    // TdmMng::ITdmOwner impl
    private:

        void TdmFrameRead(TdmMng::TdmReadWrapper &block) 
        {
            using DevIpTdm::BfLed;

            byte sign = block.GetFrame(0, 0)[0];

            BfLed::SetColor( (sign == 0xa6) ? DevIpTdm::GREEN : DevIpTdm::RED );	
        }

        void TdmFrameWrite(TdmMng::TdmWriteWrapper &block) 
        {
            byte tsMap[] = {0x00, 0x00, 0x35, 0xCA, 0x00, 0, 0, 0, 
                            0,    0,    0,    0,    0,    0, 0, 0};

            int blockSize = block.BlockSize();
            ESS_ASSERT(blockSize == 160);

            tsMap[6]  = 0;
            tsMap[14] = 0;

            for(int frame = 0; frame < blockSize; ++frame)
            {
                word *pFrame = block.GetFrame(frame, 0);

                for(int i = 0; i < sizeof(tsMap); ++i)
                {
                    pFrame[i] = tsMap[i];
                }
            }
        }

    public:

        AozLwBoard(iCore::MsgThread &thread, 
            iLogW::ILogSessionCreator &log,
            std::string logPrefix,
            const iCmp::BfInitDataAoz &data,
            TdmMng::TdmHAL &hal) : 
            iCore::MsgObject(thread),
            m_setup(data),
            m_hal(hal),
            m_counter(0)
        {
            ESS_ASSERT( DevIpTdm::AozInit::InitDone() );

            // register in HAL
            ESS_ASSERT(data.SignSportNum   == 0);
            ESS_ASSERT(data.SignSportChNum == 1);

            m_hal.RegisterTdm(data.SignSportNum, data.SignSportChNum, 
                *this, data.UseAlaw /* dummi */);
        }

        ~AozLwBoard()
        {
            m_hal.UnregTdm(*this);
        }

        void GetTdmSnapshot(std::vector<byte> &tdmIn, 
            std::vector<byte> &tdmOut)
        {
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

            // count
            ++m_counter;

            DevIpTdm::BfLed::Set(0, (m_counter & (1 << 4)));
        }

        void BlockCollision()
        {
        }

        void SetCofidecSetup(int chMask, const iCmp::AbCofidecParams &params)
        {
        }

        void SetLineParams(int chMask, const iCmp::AbLineParams &params)
        {
        }

        void Command(int chMask, const std::string &cmdName, const std::string &cmdParams)
        {
        }

        bool HasEvents() const
        {
            return false;
        }

        AozEvent PopEvent()
        {
            ESS_UNIMPLEMENTED;

            AozEvent e(0, "", "");
            return e;
        }

    };
    
    
    
}  // namespace DrvAoz

#endif

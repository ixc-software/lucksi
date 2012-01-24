#ifndef __CHANNELECHO__
#define __CHANNELECHO__

#include "Utils/ErrorsSubsystem.h"
#include "Utils/IntToString.h"
#include "BfDev/BfUartSimpleCore.h"
#include "BfDev/SysProperties.h"

namespace TdmMng
{
    using Platform::byte;
    using Platform::int16;
    using Platform::dword;
    using std::vector;

    enum
    {
        CDisableEchoMath = false,  // set true for debug
    };

    class ChannelEchoDebug : boost::noncopyable
    {
        enum State { StOpen, StClosed, StSended };

        const int m_number;
        const dword m_traceFirstSamples;

        State m_state;
        vector<int16> m_rx;
        vector<int16> m_tx;
        vector<int16> m_rxFixed;

        void SendBlock(BfDev::BfUartSimpleCore &uart, const std::string &header, 
            const vector<int16> &data)
        {
            // header
            {
                std::ostringstream oss;                    
                oss << header << "[" << data.size() << "]\n";
                uart.Send( oss.str().c_str() );
            }

            // data
            for(int i = 0; i < data.size(); ++i)
            {
                char intBuff[16];
                Utils::IntToString(data.at(i), intBuff, sizeof(intBuff));
                uart.Send(intBuff);
                uart.Send(" ");
            }

            uart.Send("\n\n");
        }

    public:

        ChannelEchoDebug(int chNumber, dword traceFirstSamples) : 
          m_number(chNumber), m_traceFirstSamples(traceFirstSamples) 
        {
            ESS_ASSERT(m_traceFirstSamples > 0);

            m_state = StOpen;

            m_rx.reserve(m_traceFirstSamples);
            m_tx.reserve(m_traceFirstSamples);
            m_rxFixed.reserve(m_traceFirstSamples);
        }

        void Trace(int tx, int rx, int fixedRx)
        {
            if (m_state != StOpen) return;

            m_rx.push_back(rx);
            m_tx.push_back(tx);
            m_rxFixed.push_back(fixedRx);

            if (m_rx.size() >= m_traceFirstSamples) m_state = StClosed;
        }

        void TryFinalize()
        {
            if (m_state != StClosed) return;

            dword freq = BfDev::SysProperties::Instance().getFrequencySys();
            BfDev::BfUartSimpleCore uart(freq, 0, 115200);

            // send channel number
            {
                std::ostringstream oss;
                oss << "Echo debug, " << m_number << " ch\n";
                uart.Send( oss.str().c_str() );
            }

            // send data
            SendBlock(uart, "RX", m_rx);
            SendBlock(uart, "TX", m_tx);
            SendBlock(uart, "RX'", m_rxFixed);

            m_state = StSended;
        }

    };

    // -------------------------------------------------------
    
    class ChannelEcho : boost::noncopyable
    {
        int m_chNumber;
        iDSP::EchoCancel m_echo;
        iDSP::ICodec &m_codec;

        boost::scoped_ptr<ChannelEchoDebug> m_debug;

        std::vector<Platform::int16> m_rx, m_tx, m_result;

    public:

        ChannelEcho(int chNumber, int taps, iDSP::ICodec &codec, dword traceFirstSamples) :
          m_chNumber(chNumber), m_echo(taps), m_codec(codec)
        {
            if (traceFirstSamples > 0)
            {
                m_debug.reset( new ChannelEchoDebug(chNumber, traceFirstSamples) );
            }
        }

        // result -> rxBuff
        bool ApplyEcho(Utils::BidirBuffer &rxBuff, const vector<byte> &syncTx, 
            ITdmDataCapture *pCapture)
        {
            ESS_ASSERT(rxBuff.Size() == syncTx.size());
            int blockSize = rxBuff.Size(); 

            // capture rx and tx
            if (pCapture != 0) pCapture->SetEchoRxTx(rxBuff, syncTx);

            // decode rx and tx
            {
                if (m_rx.empty())
                {
                    m_rx.resize(blockSize);
                    m_tx.resize(blockSize);
                    m_result.resize(blockSize);
                }

                m_codec.DecodeBlock(rxBuff.Front(), &m_rx[0], blockSize);
                m_codec.DecodeBlock(&syncTx[0],     &m_tx[0], blockSize);
            }

            // echo loop
            int16 *pRx = &m_rx[0];
            int16 *pTx = &m_tx[0];
            int16 *pResult = &m_result[0];

            for(int i = 0; i < blockSize; ++i)
            {
                int16 tx = *pTx++;
                int16 rx = *pRx++;

                int16 fixedRx = CDisableEchoMath ? rx : m_echo.Process(tx, rx);                

                *pResult++ = fixedRx;

                if (m_debug)   
                {
                    m_debug->Trace(tx, rx, fixedRx);
                }
            }

            // encode m_result -> rxBuff
            m_codec.EncodeBlock(&m_result[0], rxBuff.Front(), blockSize);

            // capture fixed RX (as fixed TX, inverted)
            if (pCapture != 0) pCapture->SetEchoFixedRx(rxBuff);

            // done
            if (m_debug)
            {
                m_debug->TryFinalize();
            }

            return true;
        }

    };
    
    
}  // namespace TdmMng

#endif

#ifndef __TDMDATACAPTURE__
#define __TDMDATACAPTURE__

#include "iCmp/ChMngProto.h"

#include "ITdmDataCapture.h"
#include "TdmEvents.h"
#include "ITdmManager.h"

namespace TdmMng
{
    
    class TdmDataCapture : public ITdmDataCapture, boost::noncopyable
    {
        enum Mode
        {
            mNone,
            mEchoMode,
            mUsualMode,
            mBlocked,
        };

        ITdmManager &m_mng;
        const std::string m_devName;
        const int m_chNum;
        const int m_minBuffSize;

        Mode m_mode;

        // std::vector<byte> m_rx, m_tx, m_txFixed;
        boost::shared_ptr<TdmAsyncCaptureDataBody> m_body;

        void CreateBody()
        {
            m_body.reset( new TdmAsyncCaptureDataBody(m_devName, m_chNum, m_minBuffSize) );
        }

        void BadMode()
        {
            if (m_mode == mBlocked) return;

            std::string src = iCmp::BfTdmEvent::MakeEventSource(m_devName, m_chNum);
            m_mng.Queue().Push( 
                TdmAsyncEvent( src, iCmp::BfTdmEvent::CDataCaptureBlocked() ) 
                );
            // m_mng.PushAsyncEvent(  );
            
            m_mode = mBlocked;
        }

        void PushData()  
        {
            m_mng.Queue().Push( TdmAsyncCaptureData(m_body) );
            CreateBody();
        }

        void UsualModeTryPushData()
        {
            ESS_ASSERT(m_mode == mUsualMode);
            ESS_ASSERT(m_body->RxFixed.size() == 0);

            if (m_body->Rx.size() != m_body->Tx.size()) return;
            if (m_body->Rx.size() >= m_minBuffSize) PushData();
        }

        static int VectorAppend(std::vector<byte> &v, const void *pData, size_t size)
        {
            int prevSize = v.size();
            v.resize( prevSize + size );
            std::memcpy(&v[prevSize], pData, size);
            return v.size();
        }

        // return true if m_minBuffSize reached
        bool Append(std::vector<byte> &buff, const std::vector<byte> &data)
        {
            ESS_ASSERT( data.size() > 0 );
            return (VectorAppend(buff, &data[0], data.size()) >= m_minBuffSize);
        }

        bool Append(std::vector<byte> &buff, const Utils::BidirBuffer &data)
        {
            ESS_ASSERT( data.Size() > 0 );
            return (VectorAppend(buff, data.Front(), data.Size()) >= m_minBuffSize);
        }

    // ITdmDataCapture impl
    private:

        /*
             Первый вызов функции, который определяет режим:
             SetEchoRxTx() или SetRxData()

             SetEchoFixedRx() всегда идет после SetEchoRxTx()
             SetTxData() вызывается всегда, поэтому режим не определяет
        */

        void SetEchoRxTx(const Utils::BidirBuffer &rx, const std::vector<byte> &tx)
        {
            if (m_mode == mNone)
            {
                m_mode = mEchoMode;
            }
            else if (m_mode != mEchoMode) 
            {
                BadMode();
                return;
            }

            ESS_ASSERT(tx.size() == rx.Size());
            Append(m_body->Rx, rx);
            Append(m_body->Tx, tx);
        }

        void SetEchoFixedRx(const Utils::BidirBuffer &rxFixed)
        {
            if (m_mode != mEchoMode)
            {
                BadMode();
                return;
            }

            if (Append(m_body->RxFixed, rxFixed)) PushData();
        }

        void SetRxData(const Utils::BidirBuffer &buff)
        {
            if (m_mode == mNone)
            {
                m_mode = mUsualMode;
            }
            else if (m_mode != mUsualMode) 
            {
                BadMode();
                return;
            }

            Append(m_body->Rx, buff);
            UsualModeTryPushData();
        }

        void SetTxData(BfTdm::IUserBlockInterface &block)
        {
            if (m_mode != mUsualMode) return;

            // append 
            {
                int prevSize = m_body->Tx.size();
                int blockSize = block.GetTSBlockSize();
                m_body->Tx.resize( prevSize + blockSize );
                block.CopyTxTSBlockTo(m_chNum, &m_body->Tx[prevSize], blockSize);
            }

            UsualModeTryPushData();
        }

    public:

        TdmDataCapture(ITdmManager &mng, 
            const std::string &devName, int chNum, int minBuffSize) : 
          m_mng(mng),
          m_devName(devName),
          m_chNum(chNum), 
          m_minBuffSize(minBuffSize),
          m_mode(mNone)
        {
            CreateBody();
        }
                
    };
    
}  // namespace TdmMng

#endif

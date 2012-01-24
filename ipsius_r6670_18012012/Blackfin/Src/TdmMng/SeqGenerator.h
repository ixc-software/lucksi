#ifndef __SEQGENERATOR__
#define __SEQGENERATOR__

#include "Platform/Platform.h"

#include "ITdmGenerator.h"

namespace TdmMng
{
    using Platform::byte;

    // циклический генератор с таблицы значений
    class SeqGenerator : public ITdmGenerator
    {
        std::vector<byte> m_data;
        int m_currPos;

        void Init(const byte *pData, int size)
        {
            m_data.clear();

            m_data.reserve(size);

            for(int i = 0; i < size; ++i)
            {
                m_data.push_back(pData[i]);
            }

            m_currPos = 0;
        }

        // dstPos + chNum is unused
        void DoWrite(Utils::BidirBuffer &buff, int dstPos, int srcPos, int count, int chNum)
        {
            buff.PushBack(&m_data.at(srcPos), count);
        }

        void DoWrite(ITdmWriteWrapper &buff, int dstPos, int srcPos, int count, int chNum)
        {
            buff.Write(chNum, dstPos, &m_data.at(srcPos), count);
        }

        template<class T>
        void Write(T &buff, int size, int chNum)
        {
            int wrPos = 0;
            int dataSize = m_data.size();

            while(true)
            {
                int needWrite = size - wrPos;  // max
                int canWrite = (dataSize - m_currPos);

                if (needWrite > canWrite) needWrite = canWrite;

                ESS_ASSERT(m_currPos + needWrite <= dataSize);
                DoWrite(buff, wrPos, m_currPos, needWrite, chNum);

                m_currPos += needWrite;
                if (m_currPos >= dataSize) m_currPos = 0;

                wrPos += needWrite;
                if (wrPos >= size) break;  // complete
            }
        }


    // ITdmGenerator impl       
    public:     
        
        void WriteTo(Utils::BidirBuffer &buff, int size)
        {
            Write(buff, size, -1);
        }
        
        void WriteTo(ITdmWriteWrapper &buff, int chNum)
        {
            Write(buff, buff.BlockSize(), chNum);
        }

        bool Completed()
        {
            return false;
        }

                
    public:     
        
        SeqGenerator(const std::vector<byte> &data)
        {
            ESS_ASSERT(data.size() > 0);

            Init(&data[0], data.size());
        }

        SeqGenerator(const byte *pData, int size)
        {
            ESS_ASSERT(size > 0);

            Init(pData, size);
        }
        
    };

    // -------------------------------------------------

    // 1 KHz A-low
    class SeqGen1KHzALow : public SeqGenerator
    {
        enum { CDataSize = 8 };
        
        static const byte* Data()
        {
            static byte data[CDataSize] = {0x3a, 0xd5, 0xba, 0xa3, 0xba, 0xd5, 0x3a, 0x23};
            return  data;
        }

    public:

        SeqGen1KHzALow() : SeqGenerator(Data(), CDataSize)
        {
        }
    };
    
    
}  // namespace TdmMng

#endif

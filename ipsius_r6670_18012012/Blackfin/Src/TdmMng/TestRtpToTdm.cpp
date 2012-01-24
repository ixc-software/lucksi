#include "stdafx.h"

#include "Utils/Random.h"
#include "Utils/MemoryPool.h"
#include "Utils/BidirBufferCreator.h"
#include "TdmMng/RtpToTdmBuff.h"
#include "TdmMng/TdmWriteWrapperEmul.h"

#include "TestRtpToTdm.h"

namespace
{
    using namespace TdmMng;
    using std::vector;
    using Platform::byte;

    // emulate RtpSession
    class Writer : boost::noncopyable
    {
        RtpToTdmBuff& m_buffer;

        Utils::BidirBufferCreator<> m_pool;

        /*
        Utils::FixedMemBlockPool<> m_buffPool;
        Utils::FixedMemBlockPool<> m_objPool; */

        const int m_maxSize;

        static int AlignSize(int size)  // for platform align
        {
            return ((size / 4) + 1) * 4;
        }

    public:
        Writer(RtpToTdmBuff& buffer, int maxSize) : 
            m_buffer(buffer),
            m_pool(AlignSize(maxSize), 0, 0, Utils::FixedMemBlockPool<>::CNoLimitedNew), 
            m_maxSize(maxSize)
        {
        }

        void WriteZeroBytes(int byteCount)
        {
            const vector<byte> fictiveData(byteCount, 0);
            WriteData(fictiveData);
        }

        // записывает данные одним пакетом
        void WriteData(const vector<byte>& data)
        {   
            ESS_ASSERT(data.size() <= m_maxSize);
            ESS_ASSERT(data.size() > 0);

            Utils::BidirBuffer *pBuff = m_pool.CreateBidirBuff(); //   new(m_objPool) Utils::BidirBuffer(m_buffPool, 0);

            pBuff->PushBack(&data.front(), data.size());
                
            m_buffer.PutData(pBuff);            
        }
    };

    //-------------------------------------------------------------------------------------

    class Reader : boost::noncopyable
    {
        const int m_readSize;
        TdmMng::ITdmSource& m_src;
        
    public:
        Reader(TdmMng::ITdmSource& buffer, int readSize)
            : m_readSize(readSize),
            m_src(buffer)
        {}

        bool Read()
        {
            vector<byte> fictive;
            return ReadTo(fictive);
        }
        
        bool ReadTo(vector<byte>& data)
        {   
            TdmMng::TdmWriteWrapperEmul dataAgent(m_readSize, data);
            return m_src.WriteToBlock(dataAgent);
        }

    };

    // -------------------------------------------------------------------------------------

    void InitRtpRecvBufferProfile(RtpRecvBufferProfile &prof)
    {
        prof.MinBufferingDepth   = 100;
        prof.MaxBufferingDepth   = 200;
        prof.UpScalePercent      = 105;
        prof.QueueDepth          = 32;
        prof.DropCount           = 1;
    }

    // -------------------------------------------------------------------------------------

    void StartBufferingTest()
    {
        // test cfg:
        const int readSize = 16;
        const int minBuffDepth = 20 * readSize;        

        RtpRecvBufferProfile prof;
        InitRtpRecvBufferProfile(prof);
        prof.MinBufferingDepth = minBuffDepth;
        prof.MaxBufferingDepth = minBuffDepth;
        prof.QueueDepth = minBuffDepth; // 1packet/1byte
        prof.UpScalePercent = 101; // any

        RtpToTdmBuff buffer(prof, 6); // time-slot any
        buffer.Start();

        Writer writer(buffer, 4);
        Reader reader(buffer, readSize);
        for (int i = 0; i < minBuffDepth - 1; ++i)
        {            
            writer.WriteZeroBytes(1);
            TUT_ASSERT( reader.Read() == false ); // в состоянии буфферизации вычитывание невозможно..
            TUT_ASSERT( buffer.getLastStat().GoesBufferingCount == 0 ); // .. однако проскальзыванием это не считается
        }
        writer.WriteZeroBytes(1); // в буффер записанно MinBufferingDepth байт, те теперь вычитывание разрешено:
        TUT_ASSERT( reader.Read() == true );
        buffer.Stop();
    }

    // -------------------------------------------------------------------------------------

    /*
            Writer -- использовать пул для буферов
            Writer должен быть выше RtpToTdmBuff - ?
            ~RtpToTdmBuff -- очищает буфера из очереди - ?
    */

    void WriteOnlyTest()
    {
        const int CPackSize = 160;

        RtpRecvBufferProfile prof;
        InitRtpRecvBufferProfile(prof);

        prof.MinBufferingDepth   = 128;
        prof.MaxBufferingDepth   = 2 * 1024;
        prof.UpScalePercent      = 150;
        prof.QueueDepth          = 32;
        prof.DropCount           = prof.QueueDepth / 2;

        prof.DropCount = prof.QueueDepth / 2;

        RtpToTdmBuff buffer(prof, 6); // time-slot any
        buffer.Start();

        Writer writer(buffer, CPackSize);

        for(int i = 0; i < prof.QueueDepth * 32; ++i)
        {
            writer.WriteZeroBytes(CPackSize);
        }

        // отдать буфера в writer - ?!!
        buffer.Stop();  

    }

    // -------------------------------------------------------------------------------------

    void SlipTest()
    {
        const int upDepthStepCount = 10;
        const int upDepthStepSize = 10; // %        

        RtpRecvBufferProfile prof;
        InitRtpRecvBufferProfile(prof);
        prof.MinBufferingDepth = 10; // any

        // вычисляем макс. глубину исходя из заданного кол-ва шагов
        prof.MaxBufferingDepth = prof.MinBufferingDepth;
        for (int i = 0; i < upDepthStepCount; ++i)
            prof.MaxBufferingDepth += prof.MaxBufferingDepth * upDepthStepSize / 100;       

        prof.QueueDepth = prof.MaxBufferingDepth;  // 1packet/1byte
        prof.UpScalePercent = 100 + upDepthStepSize;
        RtpToTdmBuff buffer(prof, 6); // time-slot any
        buffer.Start();
        
        int currDepth = prof.MinBufferingDepth;
        Writer writer(buffer, prof.MaxBufferingDepth);
        for (int step = 0; step <= upDepthStepCount; step++)
        {                        
            writer.WriteZeroBytes(currDepth); // заполняем на тек глубину

            Reader reader(buffer, 1);
            // успешно вычитываем всё
            for (int readCount = 0; readCount < currDepth; readCount++)
            {
                TUT_ASSERT(reader.Read() == true);
            }

            TUT_ASSERT (buffer.getLastStat().GoesBufferingCount == step);
            // проскальзывание
            TUT_ASSERT(reader.Read() == false);            
            TUT_ASSERT (buffer.getLastStat().GoesBufferingCount == step + 1);

            currDepth += currDepth * upDepthStepSize / 100;
        } 

        buffer.Stop();
    }

    //-------------------------------------------------------------------------------------

    void OverflowTest()
    {
        RtpRecvBufferProfile prof;
        InitRtpRecvBufferProfile(prof);
        prof.MinBufferingDepth = 100;
        prof.MaxBufferingDepth = 200; 
        prof.QueueDepth = 100;        

        RtpToTdmBuff buffer(prof, 6); // time-slot any
        buffer.Start();

        const int maxWriteSize = 16;
        Writer writer(buffer, maxWriteSize);

        // заполняем до предела
        for (int i = 0; i < prof.QueueDepth; ++i)
        {	
            writer.WriteZeroBytes(1);
        }

        // переполнение
        {                        
            // при переполнении буфер должен был очиститься и оставаясь в режиме,
            // когда из буфера разрешено чтение, пропускать данные
            vector<byte> somsingTxData;            
            for (int i = 0; i < maxWriteSize; ++i)            
                somsingTxData.push_back(i * 10);
            writer.WriteData(somsingTxData); // переполнение

            TUT_ASSERT (buffer.getLastStat().OverflowFifoCount == 1);                                    

            vector<byte> rxData;            
            Reader(buffer, somsingTxData.size()).ReadTo(rxData);
            TUT_ASSERT(rxData == somsingTxData);                        

            TUT_ASSERT (Reader(buffer, 1).Read() == false);
            TUT_ASSERT (buffer.getLastStat().OverflowFifoCount == 1); //проверка ведения статистики
        }    

        buffer.Stop();

    }

    //-------------------------------------------------------------------------------------

    void DataExchangeTest()
    {        
        const int avgPackSize = 256;
        const int deltaPackSize = 20;
        const int maxRwCycle = 500;
        const int readSize = avgPackSize + 10;

        RtpRecvBufferProfile prof;
        InitRtpRecvBufferProfile(prof);
        prof.MinBufferingDepth = 10 * readSize;
        prof.MaxBufferingDepth = prof.MinBufferingDepth * 2; 
        prof.QueueDepth = 1000;
        prof.UpScalePercent = 115;

        RtpToTdmBuff buffer(prof, 6); // time-slot any
        buffer.Start();

        // собственно тест
        Writer writer(buffer, avgPackSize + deltaPackSize);
        vector<byte> somethingTxData; // отправляемые данные накапливаем здесь        
        vector<byte> rxData; // принимаемые данные накапливаем здесь

        bool lastReadComplete;
        for (int rwCycle = 0; rwCycle < maxRwCycle; ++rwCycle)
        {
            Utils::Random rnd(Platform::GetSystemTickCount());        

            Reader reader(buffer, readSize);

            // write data
            {
                vector<byte> packet;
                int packetSize = avgPackSize + rnd.Next(2 * deltaPackSize) - deltaPackSize;                            
                for (int i = 0; i < packetSize; ++i)            
                    packet.push_back(i * 10);

                writer.WriteData(packet);

                for(int i = 0; i < packet.size(); ++i)
                {
                    somethingTxData.push_back( packet.at(i) );
                }

                // std::copy ( packet.begin(), packet.end(), std::inserter(somethingTxData, somethingTxData.end()) ); // add packet to somethingTxData
            }

            // read data
            lastReadComplete = reader.ReadTo(rxData);
        } 

        ESS_ASSERT( rxData.size() != 0 );        

        int diffSize = somethingTxData.size() - rxData.size();

        if (lastReadComplete)
            Reader(buffer, diffSize).ReadTo(rxData);
        else
            for (int i = 0; i < diffSize; ++i)
                somethingTxData.pop_back();          

        TUT_ASSERT(somethingTxData == rxData);

        buffer.Stop();                
        //std::cout <<  buffer.getLastStat().getAsString() << "\n";
    }

} // namespace

//-------------------------------------------------------------------------------------

namespace TestBuffering
{

    void TestRtpToTdmBuff()
    {        
        std::cout << "Test RtpToTdmBuff started..." << std::endl;

        WriteOnlyTest();

        StartBufferingTest();
        SlipTest();
        // OverflowTest();        
        DataExchangeTest();
               
        std::cout << "...test RtpToTdmBuff completed." << std::endl;
    }

} // namespace TestBuffering


#ifndef __MSWAVE__
#define __MSWAVE__

#include "Platform/Platform.h"
#include "Utils/ErrorsSubsystem.h"

namespace Utils
{
    using Platform::dword;
    using Platform::word;
    using Platform::byte;
	
	// Class for work with .wav audio files
    class MsWaveOld : boost::noncopyable
	{
    public:

        enum CodecFormat
        {
            Pcm     = 1,
            G711a   = 6,
            G711u   = 7,
        };

        struct FmtChunk
        {
            word  Format;
            word  Channels;
            dword SampleRate;
            dword ByteRate;
            word  BlockAlign;
            word  BitsPerSample;

            FmtChunk()
            {
                // mark as empty
                Format = 0;
                Channels = 0;  
            }
        };

        static const char* CDataChunk() { return "data"; }
        static const char* CRiffChunk() { return "RIFF"; }
        static const char* CWaveID()    { return "WAVE"; }
        static const char* CFmtChunk()  { return "fmt "; }

        ESS_TYPEDEF(MsWaveException);
        ESS_TYPEDEF_FULL(OpenError, MsWaveException);
        ESS_TYPEDEF_FULL(BadFormat, MsWaveException);

        MsWaveOld(QString fileName) // can throw MsWaveException            
        {
            QFile f(fileName);
            if (!f.open(QIODevice::ReadOnly)) ESS_THROW(OpenError);

            QByteArray data = f.readAll();
            QDataStream ds(data);
            LoadFromStream(ds);
        }

        MsWaveOld(const QByteArray &data)
        {
            QDataStream ds(data);
            LoadFromStream(ds);
        }

        ~MsWaveOld()
        {
            ESS_ASSERT(m_iteratorUsageCount == 0);
        }

        const FmtChunk& Format() const
        {
            ESS_ASSERT(m_fmt.Format > 0);

            return m_fmt;
        }


        template<class TCont>
        static QByteArray MakeWavStreamOneCh(CodecFormat format, int sampleRate, int bytesPerSample, 
            /* const */ TCont &cont)
        {
            return MakeWavStreamOneCh(format, sampleRate, bytesPerSample, cont.begin(), cont.end());
        }

        template<class TIter>
        static QByteArray MakeWavStreamOneCh(CodecFormat format, int sampleRate, int bytesPerSample, 
            TIter first, TIter last)
        {           
            // make data chunk body
            QByteArray dataChunk;

            int dataSize = (last - first) * bytesPerSample;
            dataChunk.reserve(128 + dataSize);

            QDataStream ds(&dataChunk, QIODevice::WriteOnly);
            ds.setByteOrder(QDataStream::LittleEndian);

            WriteBytes(ds, CDataChunk());
            WriteDword(ds, dataSize);

            while(first != last)
            {
                WriteSample(ds, *first, bytesPerSample);

                ++first;
            }

            // make wav stream
            return MakeWavStream(format, sampleRate, bytesPerSample, 1, dataChunk);
        }

        template<class TContCh0, class TContCh1>
        static QByteArray MakeWavStreamTwoCh(CodecFormat format, int sampleRate, int bytesPerSample, 
            /* const */ TContCh0 &contCh0, /* const */ TContCh1 &contCh1)
        {
            return MakeWavStreamTwoCh(format, sampleRate, bytesPerSample, 
                contCh0.begin(), contCh0.end(),
                contCh1.begin(), contCh1.end());
        }

        template<class TIterCh0, class TIterCh1>
        static QByteArray MakeWavStreamTwoCh(CodecFormat format, int sampleRate, int bytesPerSample, 
            TIterCh0 ch0First, TIterCh0 ch0Last, TIterCh1 ch1First, TIterCh1 ch1Last)
        {           
            // make data chunk body
            QByteArray dataChunk;

            int ch0Samples = ch0Last - ch0First;
            int ch1Samples = ch1Last - ch1First;
            ESS_ASSERT( ch0Samples == ch1Samples );

            int dataSize = ch0Samples * 2 * bytesPerSample;
            dataChunk.reserve(128 + dataSize);

            QDataStream ds(&dataChunk, QIODevice::WriteOnly);
            ds.setByteOrder(QDataStream::LittleEndian);

            WriteBytes(ds, CDataChunk());
            WriteDword(ds, dataSize);

            while(ch0First != ch0Last)
            {
                WriteSample(ds, *ch0First, bytesPerSample);
                WriteSample(ds, *ch1First, bytesPerSample);

                ++ch0First;
                ++ch1First;
            }

            // make wav stream
            return MakeWavStream(format, sampleRate, bytesPerSample, 2, dataChunk);
        }


    private:

        enum 
        { 
            CChunkSize = 4,
            CFmtChunkMinSize = 16, 
        };

        static QByteArray MakeWavStream(CodecFormat format, int sampleRate, int bytesPerSample, int channels,
            QByteArray dataChunk)
        {
            QByteArray res;

            int totalSize = 4;                       /* WAVE */
            totalSize    += 8 + CFmtChunkMinSize;    /* fmt  + fmt size */
            totalSize    += dataChunk.length();      /* data chunk */

            res.reserve(totalSize);
            QDataStream ds(&res, QIODevice::WriteOnly);
            ds.setByteOrder(QDataStream::LittleEndian);

            // RIFF + WAVE
            {
                WriteBytes(ds, CRiffChunk());
                WriteDword(ds, totalSize);
                WriteBytes(ds, CWaveID());
            }

            // fmt
            {
                WriteBytes(ds, CFmtChunk());
                WriteDword(ds, CFmtChunkMinSize);

                WriteWord(ds, format);
                WriteWord(ds, channels); 
                WriteDword(ds, sampleRate);
                WriteDword(ds, sampleRate * bytesPerSample * channels);
                WriteWord(ds, 1);
                WriteWord(ds, bytesPerSample * 8);
            }

            // data
            WriteBytes(ds, dataChunk);

            return res;
        }

        template<class TVal>
        static void WriteSample(QDataStream &ds, TVal val, int bytesPerSample)
        {
            int writeCount = 0;

            while(writeCount < bytesPerSample)
            {
                quint8 b = (val >> (8 * writeCount));
                ds << b;

                writeCount++;
            }
        }

        static void WriteBytes(QDataStream &ds, QByteArray data)
        {
            int writen = ds.writeRawData(data.data(), data.length()); 

            ESS_ASSERT( writen == data.length() );
        }

        static void WriteWord(QDataStream &ds, word val)
        {
            quint16 v = val;
            ds << v;
        }

        static void WriteDword(QDataStream &ds, dword val)
        {
            quint32 v = val;
            ds << v;
        }

        static QByteArray ReadBytes(QDataStream &ds, int size)
        {
            ESS_ASSERT(size > 0);

            QByteArray res;
            res.resize(size);

            if (ds.readRawData(res.data(), size) != size) return "";

            return res;
        }

        static dword ReadDword(QDataStream &ds)
        {
            quint32 res;
            ds >> res;
            return res;
        }

        static dword ReadWord(QDataStream &ds)
        {
            quint16 res;
            ds >> res;
            return res;
        }

        // return full chunk size in bytes, -1 if error
        int ReadFmtChunk(QDataStream &ds, /* out */ FmtChunk &fmt)
        {
            QByteArray chName = ReadBytes(ds, CChunkSize);
            if (chName != CFmtChunk()) return -1;

            int size = ReadDword(ds);
            if (size < CFmtChunkMinSize) return -1;

            fmt.Format          = ReadWord(ds);
            fmt.Channels        = ReadWord(ds);
            fmt.SampleRate      = ReadDword(ds);
            fmt.ByteRate        = ReadDword(ds);
            fmt.BlockAlign      = ReadWord(ds);
            fmt.BitsPerSample   = ReadWord(ds);

            // read extra bytes 
            if (size > CFmtChunkMinSize) ReadBytes(ds, size - CFmtChunkMinSize);

            return size + 8;
        }

        static QByteArray SearchAndReadData(QDataStream &ds, int totalSize)
        {
            QByteArray data;

            while(totalSize > 0)
            {
                QByteArray chName = ReadBytes(ds, CChunkSize);
                dword chSize = ReadDword(ds);

                if (chName == CDataChunk())
                {
                    data = ReadBytes(ds, chSize);
                }
                else 
                {
                    // skip chunk
                    ReadBytes(ds, chSize);
                }

                totalSize -= (8 + chSize);
                if (ds.status() != QDataStream::Ok) ESS_THROW(BadFormat);
            }

            return data;
        }

        void LoadFromStream(QDataStream &ds)
        {
            m_iteratorUsageCount = 0;
            ds.setByteOrder(QDataStream::LittleEndian);

            int totalSize = 0;

            // read main chunk
            {
                QByteArray chName = ReadBytes(ds, CChunkSize);
                if (chName != CRiffChunk()) ESS_THROW(BadFormat);

                totalSize = ReadDword(ds);

                QByteArray riffType = ReadBytes(ds, CChunkSize);
                if (riffType != CWaveID()) ESS_THROW(BadFormat);
                totalSize -= 4;
            }

            // read "fmt" sub-chunk
            {
                int fmtSize = ReadFmtChunk(ds, m_fmt);
                if (fmtSize <= 0) ESS_THROW(BadFormat);
                totalSize -= fmtSize;
            }

            // search and read "data" sub-chunk
            m_data = SearchAndReadData(ds, totalSize);

            // data chunk not found
            if (m_data.length() == 0) ESS_THROW(BadFormat);
        }

        friend class WaveChIteratorOld;

        void IteratorLock()
        {
            m_iteratorUsageCount++;
        }

        void IteratorFree()
        {
            ESS_ASSERT(m_iteratorUsageCount > 0);

            m_iteratorUsageCount--;            
        }

        FmtChunk    m_fmt;
        QByteArray  m_data;  	    
        int         m_iteratorUsageCount;
	};

    // -----------------------------------------------------------

    // Channel data read iterator
    class WaveChIteratorOld : boost::noncopyable
    {
        MsWaveOld &m_owner;
        int m_chNum;
        int m_sampleSize; // in bytes
        int m_samplesCount;
        int m_signMask;

        int m_pos;  // position (current sample)

    public:

        WaveChIteratorOld(MsWaveOld &owner, int chNum) : 
          m_owner(owner), m_chNum(chNum)
        {
            m_owner.IteratorLock();

            ESS_ASSERT(m_chNum < m_owner.m_fmt.Channels);

            int bps = m_owner.m_fmt.BitsPerSample;
            ESS_ASSERT(bps % 8 == 0);
            m_sampleSize = bps / 8;

            m_samplesCount = m_owner.m_data.length() / m_owner.m_fmt.Channels / m_sampleSize;

            {
                m_signMask = 0;
                int startBit = bps;
                int totalBits = sizeof(int) * 8;

                for(int i = startBit; i < totalBits; ++i)
                {
                    m_signMask |= (1UL << i);
                }
            }
            

            First();
        }

        ~WaveChIteratorOld()
        {
            m_owner.IteratorFree();
        }

        int Get() const
        {
            ESS_ASSERT(m_pos < m_samplesCount);

            int indx = (m_pos * m_sampleSize * m_owner.m_fmt.Channels) + (m_chNum * m_sampleSize);

            // read sample
            int res = 0;
            int readBytes = 0;

            while(readBytes < m_sampleSize)
            {
                byte val = m_owner.m_data.at(indx + readBytes);
                res |= (val << (8 * readBytes));
                readBytes++;

                if (readBytes == m_sampleSize)
                {
                    if ((val & 0x80) != 0) res |= m_signMask;
                }
            }
                       
            return res;
        }

        void First()
        {
            m_pos = 0;
        }

        void Next()
        {
            m_pos++;
        }

        bool Eof() const
        {
            return (m_pos >= m_samplesCount);
        }

        int Pos() const
        {
            return m_pos;
        }

        void Pos(int val)
        {
            ESS_ASSERT(val < m_samplesCount);

            m_pos = val;
        }

        int Size() const
        {
            return m_samplesCount;
        }

    };

	// --------------------------------------------------------

    void MsWaveOldTest();

	
}  // namespace Utils


#endif

#ifndef __MSWAVE__
#define __MSWAVE__

#include "Platform/Platform.h"

#include "ErrorsSubsystem.h"
#include "UtilsClassStr.h"

namespace Utils
{
    using Platform::dword;
    using Platform::word;
    using Platform::byte;

    // ------------------------------------------------------------

    class MsWaveBase
    {

    protected:

        enum 
        { 
            CChunkSize          = 4,
            CFmtChunkMinSize    = 16, 
        };

        UTILS_CLASS_STR(CDataChunk,     "data");
        UTILS_CLASS_STR(CRiffChunk,     "RIFF");
        UTILS_CLASS_STR(CWaveID,        "WAVE");
        UTILS_CLASS_STR(CFmtChunk,      "fmt ");
        UTILS_CLASS_STR(CFactChunk,     "fact");

    public:

        enum CodecFormat
        {
            Pcm     = 1,
            G711a   = 6,
            G711u   = 7,
        };

        ESS_TYPEDEF(MsWaveException);
        ESS_TYPEDEF_FULL(BadFormat, MsWaveException);

    };

    // ------------------------------------------------------------

	// Class for work with .wav audio files
    class MsWaveReader : MsWaveBase, boost::noncopyable
	{
    public:

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

        ESS_TYPEDEF_FULL(OpenError, MsWaveException);

        MsWaveReader(QString fileName) // can throw MsWaveException            
        {
            QFile f(fileName);
            if (!f.open(QIODevice::ReadOnly)) ESS_THROW(OpenError);

            QByteArray data = f.readAll();
            QDataStream ds(data);
            LoadFromStream(ds);
        }

        MsWaveReader(const QByteArray &data)
        {
            QDataStream ds(data);
            LoadFromStream(ds);
        }

        ~MsWaveReader()
        {
            ESS_ASSERT(m_iteratorUsageCount == 0);
        }

        const FmtChunk& Format() const
        {
            ESS_ASSERT(m_fmt.Format > 0);

            return m_fmt;
        }

        // Channel data read iterator
        // TODO: create from MsWaveReader method, copyable
        class Iterator : boost::noncopyable
        {
            MsWaveReader &m_owner;
            int m_chNum;
            int m_sampleSize; // in bytes
            int m_samplesCount;
            int m_signMask;

            int m_pos;  // position (current sample)

        public:

            Iterator(MsWaveReader &owner, int chNum) : 
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

              ~Iterator()
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

    private:

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
                    int offset = ds.device()->pos() - 8;  // debug
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

        // friend class WaveChIterator;

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

    // --------------------------------------------------------

    class MsWaveWriter : MsWaveBase, boost::noncopyable
    {
        boost::shared_ptr<QIODevice> m_device;
        QDataStream m_stream;

        // data for header and chuhk
        int m_dataSize;  // total raw samples data size in bytes
        CodecFormat m_format;
        int m_chCount; 
        int m_sampleRate;
        int m_bytesPerSample;

        

        enum
        {
            CDataSubChunkHeaderSize = 4 + 4,
            CFactSubChunkHeaderSize = 4 + 4 + 4,
        };

        template<class T>
        void AddThruIter1x(const T *pBegin, const T *pEnd)
        {
            while(pBegin != pEnd)
            {
                Add(*pBegin++);
            }
        }

        template<class T>
        void AddThruIter2x(const T *pBegin0, const T *pEnd0, 
                           const T *pBegin1, const T *pEnd1)
        {
            while(pBegin0 != pEnd0)
            {
                Add(*pBegin0++, *pBegin1++);
            }
        }

        void WriteFactChunk()
        {
            WriteBytes(m_stream, CFactChunk());
            WriteDword(m_stream, 4);
            WriteDword(m_stream, m_dataSize / m_chCount / m_bytesPerSample);
        }

        void WriteWavChunk()  // without data
        {            
            WriteBytes(m_stream, CDataChunk());
            WriteDword(m_stream, m_dataSize); // dataSize = chSamples * chCount * bytesPerSample

            // + data
        }

        void WriteWavHeader()
        {
            int totalSize = 4;                                      // WAVE
            totalSize    += 8 + CFmtChunkMinSize;                   // fmt  + fmt size 
            totalSize    += CFactSubChunkHeaderSize + 
                            CDataSubChunkHeaderSize + m_dataSize;   // fact + data chunk 

            // RIFF + WAVE
            {
                WriteBytes(m_stream, CRiffChunk());
                WriteDword(m_stream, totalSize);
                WriteBytes(m_stream, CWaveID());
            }

            // fmt
            {
                WriteBytes(m_stream, CFmtChunk());
                WriteDword(m_stream, CFmtChunkMinSize);

                WriteWord(m_stream, m_format);
                WriteWord(m_stream, m_chCount); 
                WriteDword(m_stream, m_sampleRate);
                WriteDword(m_stream, m_sampleRate * m_bytesPerSample * m_chCount);
                WriteWord(m_stream, m_chCount * m_bytesPerSample);
                WriteWord(m_stream, m_bytesPerSample * 8);
            }

            // + data chunk
        }

        void WriteHeaders()
        {
            m_device->seek(0);
            WriteWavHeader();
            WriteFactChunk();
            WriteWavChunk();
        }

        void Init(boost::shared_ptr<QIODevice> device, 
                  CodecFormat format, int chCount, int sampleRate)
        {
            m_device = device;
            m_stream.setDevice( m_device.get() );

            m_stream.setByteOrder(QDataStream::LittleEndian);

            m_dataSize = 0;
            m_format = format;
            m_chCount = chCount; 
            m_sampleRate = sampleRate;
            m_bytesPerSample = BytesPerSampleFromFormat(format);

            if (m_bytesPerSample < 1) ESS_THROW(BadFormat);

            // write (reserve) headers
            WriteHeaders();
        }

        MsWaveWriter(boost::shared_ptr<QIODevice> device, 
            CodecFormat format, int chCount, int sampleRate) 
        {
            Init(device, format, chCount, sampleRate);
        }

    // static stuff 
    private:

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

            if(writen != data.length()) ESS_THROW(IoError);
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

        static int BytesPerSampleFromFormat(CodecFormat format)
        {
            if (format == Pcm) return 2;
            if ((format == G711u) || (format == G711a)) return 1;
            return -1;
        }

    public:

        ESS_TYPEDEF_FULL(CantOpenFileForWrite, MsWaveException);
        ESS_TYPEDEF_FULL(IoError,              MsWaveException);

        MsWaveWriter(QString fileName, CodecFormat format, int chCount, int sampleRate) 
        {
            boost::shared_ptr<QFile> file(new QFile(fileName));
            if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) 
            {
                ESS_THROW(CantOpenFileForWrite);
            }

            Init(boost::shared_ptr<QIODevice>(file), format, chCount, sampleRate);
        }

        ~MsWaveWriter()
        {
            // write real headers
            WriteHeaders();

            m_device->close();
        }

        void Add(int ch0)
        {
            ESS_ASSERT(m_chCount == 1);

            WriteSample(m_stream, ch0, m_bytesPerSample);
            m_dataSize += m_bytesPerSample;
        }

        void Add(int ch0, int ch1)
        {
            ESS_ASSERT(m_chCount == 2);

            WriteSample(m_stream, ch0, m_bytesPerSample);
            WriteSample(m_stream, ch1, m_bytesPerSample);

            m_dataSize += (m_bytesPerSample * 2);
        }

        template<class T>
        void Add(const std::vector<T> &ch0)
        {
            if (ch0.empty()) return;

            const T *pBegin = &ch0[0];
            AddThruIter1x(pBegin, pBegin + ch0.size());
        }

        template<class T>
        void Add(const std::vector<T> &ch0, const std::vector<T> &ch1)
        {
            ESS_ASSERT(ch0.size() == ch1.size());
            if (ch0.empty()) return;

            const T *pBegin0 = &ch0[0];
            const T *pBegin1 = &ch1[0];

            AddThruIter2x(pBegin0, pBegin0 + ch0.size(), 
                          pBegin1, pBegin1 + ch1.size());
        }

    // static stuff
    public:

        template<class TCont>
        static QByteArray MakeWavStreamOneCh(CodecFormat format, int sampleRate, 
            /* const */ TCont &cont)
        {
            return MakeWavStreamOneCh(format, sampleRate, cont.begin(), cont.end());
        }

        template<class TIter>
        static QByteArray MakeWavStreamOneCh(CodecFormat format, int sampleRate, 
            TIter first, TIter last)
        {           
            boost::shared_ptr<QBuffer> buff( new QBuffer() );
            buff->open(QIODevice::WriteOnly);

            {
                MsWaveWriter wr(buff, format, 1, sampleRate);

                while(first != last)
                {
                    wr.Add(*first++);
                }
            }

            return buff->buffer();
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
            int ch0Samples = ch0Last - ch0First;
            int ch1Samples = ch1Last - ch1First;
            ESS_ASSERT( ch0Samples == ch1Samples );

            boost::shared_ptr<QBuffer> buff( new QBuffer() );
            buff->open(QIODevice::WriteOnly);

            {
                MsWaveWriter wr(buff, format, 2, sampleRate);

                while(ch0First != ch0Last)
                {
                    wr.Add(*ch0First++, *ch1First++);
                }
            }

            return buff->buffer();
        }

    };

	// --------------------------------------------------------

    void MsWaveTest();

	
}  // namespace Utils


#endif

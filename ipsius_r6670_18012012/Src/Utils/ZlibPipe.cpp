#include "stdafx.h"

#include "ZlibPipe.h"

#include "zlib/zlib.h"

// ---------------------------------------

namespace Utils
{
    class ZlibDeflate::ZStream : boost::noncopyable
    {
        IBiWriterStream &m_output;
        std::vector<Platform::byte> m_buff;

        z_stream m_stream;
        bool m_finalize;

    public:

        ZStream(IBiWriterStream &output, int buffSize) :
          m_output(output),
          m_buff(buffSize, 0),
          m_finalize(false)
        {
            m_stream.zalloc = Z_NULL;
            m_stream.zfree = Z_NULL;
            m_stream.opaque = Z_NULL;

            int level = Z_DEFAULT_COMPRESSION; // Z_BEST_COMPRESSION; // Z_DEFAULT_COMPRESSION
            int ret = deflateInit(&m_stream, level);
            if (ret != Z_OK) ESS_THROW_MSG(ZlibException, "deflateInit");
        }

        void Deflate(const void *pData, int dataSize, bool finalize)
        {
            ESS_ASSERT(pData != 0);
            ESS_ASSERT(dataSize > 0);
            ESS_ASSERT(!m_buff.empty());
            ESS_ASSERT(!m_finalize);

            m_finalize = finalize;
            int flushMode = finalize ? Z_FINISH : Z_NO_FLUSH;

            // input
            m_stream.avail_in = dataSize;
            m_stream.next_in = (Bytef*)pData;

            // output loop
            while(true)
            {
                m_stream.avail_out = m_buff.size();
                m_stream.next_out = (Bytef*)&m_buff[0];

                int ret = deflate(&m_stream, flushMode);
                if (ret == Z_STREAM_ERROR) ESS_THROW_MSG(ZlibException, "deflate"); 

                int have = m_buff.size() - m_stream.avail_out;
                if (have > 0)
                {
                    m_output.Write(&m_buff[0], have);
                }

                if (have != m_buff.size())  // done
                {
                    if (finalize) ESS_ASSERT(ret == Z_STREAM_END);
                    break;
                }

            }
            
            ESS_ASSERT(m_stream.avail_in == 0);     // all input will be used 
        }

        ~ZStream()
        {
            deflateEnd(&m_stream);
        }

    };


}  // namespace Utils

// ---------------------------------------


namespace Utils
{
    class ZlibInflate::ZStream : boost::noncopyable
    {
        IBiWriterStream &m_output;
        std::vector<Platform::byte> m_buff;

        z_stream m_stream;
        bool m_finalized;

    public:

        ZStream(IBiWriterStream &output, int buffSize) :
          m_output(output),
          m_buff(buffSize, 0),
          m_finalized(false)
        {
            m_stream.zalloc = Z_NULL;
            m_stream.zfree = Z_NULL;
            m_stream.opaque = Z_NULL;
            m_stream.avail_in = 0;
            m_stream.next_in = Z_NULL;

            int ret = inflateInit(&m_stream);
            if (ret != Z_OK) ESS_THROW_MSG(ZlibException, "inflateInit");
        }

        int Inflate(const void *pData, int dataSize, bool &finalized)
        {
            ESS_ASSERT(pData != 0);
            ESS_ASSERT(dataSize > 0);
            ESS_ASSERT(!m_buff.empty());
            ESS_ASSERT(!m_finalized);

            // input
            m_stream.avail_in = dataSize;
            m_stream.next_in = (Bytef*)pData;

            // output loop
            while(true)
            {
                m_stream.avail_out = m_buff.size();
                m_stream.next_out = (Bytef*)&m_buff[0];

                int ret = inflate(&m_stream, Z_NO_FLUSH);
                if (ret == Z_STREAM_ERROR) ESS_THROW_MSG(ZlibException, "inflate"); 

                if (ret == Z_NEED_DICT || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
                {
                    ESS_THROW_MSG(ZlibException, "inflate error"); 
                }

                int have = m_buff.size() - m_stream.avail_out;
                if (have > 0)
                {
                    m_output.Write(&m_buff[0], have);
                }

                if (have != m_buff.size())  // done
                {
                    m_finalized = finalized = (ret == Z_STREAM_END);
                    int used = dataSize - m_stream.avail_in;

                    if ((used != dataSize) && !finalized)
                    {
                        ESS_THROW_MSG(ZlibException, "Block misalign!");
                    }

                    return used;
                }
                
            }

            // never happends
            return -1;
        }

        ~ZStream()
        {
            inflateEnd(&m_stream);
        }

    };


}  // namespace Utils

// ---------------------------------------

namespace Utils
{
        
    ZlibDeflate::ZlibDeflate(IBiWriterStream &output, int buffSize) :
        m_stream( new ZStream(output, buffSize) )
    {
    }

    ZlibDeflate::~ZlibDeflate()
    {
        // nothing
    }

    void ZlibDeflate::Add(const void *pData, int dataSize, bool finalize)
    {
        m_stream->Deflate(pData, dataSize, finalize);
    }

    ZlibInflate::ZlibInflate(IBiWriterStream &output, int buffSize) :
        m_stream( new ZStream(output, buffSize) )
    {
    }

    ZlibInflate::~ZlibInflate()
    {
        // nothing
    }

    int ZlibInflate::Add(const void *pData, int dataSize, bool &finalized)
    {
        return m_stream->Inflate(pData, dataSize, finalized);
    }

}  // namespace Utils


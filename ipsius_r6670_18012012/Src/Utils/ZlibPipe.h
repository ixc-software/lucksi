#ifndef __ZLIBPIPE__
#define __ZLIBPIPE__

#include "Utils/IBasicInterface.h"
#include "Utils/IBiRWStream.h"

namespace Utils
{

    ESS_TYPEDEF(ZlibException);

    // -----------------------------------------------------

    class ZlibDeflate : boost::noncopyable
    {
        class ZStream;

        boost::scoped_ptr<ZStream> m_stream;

    public:

        ZlibDeflate(IBiWriterStream &output, int buffSize = 8 * 1024);
        ~ZlibDeflate();

        void Add(const void *pData, int dataSize, bool finalize);

    };

    // -----------------------------------------------------

    class ZlibInflate : boost::noncopyable
    {
        class ZStream;

        boost::scoped_ptr<ZStream> m_stream;

    public:

        ZlibInflate(IBiWriterStream &output, int buffSize = 8 * 1024);
        ~ZlibInflate();

        // return bytes used 
        int Add(const void *pData, int dataSize, bool &finalized);

    };
    
}  // namespace Utils

#endif

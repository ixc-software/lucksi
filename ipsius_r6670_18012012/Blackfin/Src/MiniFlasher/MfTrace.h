#ifndef __MFTRACE__
#define __MFTRACE__

namespace MiniFlasher
{

    class MfTrace : boost::noncopyable
    {
        int m_counter;
        std::string m_prefix;
        int m_bytesLimit; 
        bool m_enabled;


    public:

        MfTrace(bool dataIn, int bytesLimit, bool enabled)
        {
            m_counter = 1;
            m_prefix = (dataIn) ? ">> " : "<< ";
            m_bytesLimit = bytesLimit;
            m_enabled = enabled;
        }

        void Trace(const void *pData, int size);

    };
        
}  // namespace MiniFlasher

#endif

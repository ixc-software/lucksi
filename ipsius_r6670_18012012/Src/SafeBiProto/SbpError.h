
#ifndef __SBPERROR__
#define __SBPERROR__

#include "Utils/IntToString.h"

namespace SBProto
{
    // Base class for all SafeBiProto errors
    class SbpError : boost::noncopyable
    {
        std::string m_err;
    public:
        SbpError(const std::string &err) : m_err(err)
        {
        }

        virtual ~SbpError()
        {
        }

        const std::string& ToString() const { return m_err; }
    };

    // -----------------------------------------------------------------
    // Error specializations
    
    class SbpWaitingResponceTimeout : public SbpError
    {
    public:
        SbpWaitingResponceTimeout() 
            : SbpError("SafeBiProto: waiting responce timeout")
        {
        }
    };

    // -----------------------------------------------------------------

    class SbpWaitingEndOfReceiveTimeout : public SbpError
    {
    public:
        SbpWaitingEndOfReceiveTimeout() 
            : SbpError("SafeBiProto: waiting end of receive timeout")
        {
        }
    };

    // -----------------------------------------------------------------
    
	class SbpProcessTimeout : public SbpError
	{
	public:
		SbpProcessTimeout() 
			: SbpError("Protocol timeouts are not processed or check interval is too short")
		{
		}
	};

	// -----------------------------------------------------------------
    class SbpInvalidVersion : public SbpError
    {
    public:
        SbpInvalidVersion(int recvPackProtoVer) : 
//		  SbpError(std::string("SafeBiProto: invalid protocol version ") + Utils::IntToString(wrongVersion))
		  SbpError("SafeBiProto: invalid protocol version " + Utils::IntToString(recvPackProtoVer))
        {
        }
    };

    // -----------------------------------------------------------------

    class SbpSendPackSizeIsTooBig : public SbpError
    {
        static std::string Text(int real, int expected)
        {
            std::ostringstream ss;
            ss << "SafeBiProto: sent packet size (" << real 
                << ") is bigger then maximun (" << expected << ")";
            return ss.str();
        }
    public:
        SbpSendPackSizeIsTooBig(int real, int expected) 
        : SbpError(Text(real, expected))
        {
        }
    };

    // -----------------------------------------------------------------

    class SbpRecvPackSizeIsTooBig : public SbpError
    {
        static std::string Text(int real, int expected)
        {
            std::ostringstream ss;
            ss << "SafeBiProto: received packet size (" << real 
                << ") is bigger then maximun (" << expected << ")";
            return ss.str();
        }
    public:
        SbpRecvPackSizeIsTooBig(int real, int expected) 
        : SbpError(Text(real, expected))
        {
        }
    };

    // -----------------------------------------------------------------

    class SbpRecvPackCRCCheckFailed : public SbpError
    {
    public:
        SbpRecvPackCRCCheckFailed() 
        : SbpError("SafeBiProto: received packet CRC checking failed")
        {
        }
    };


    // -----------------------------------------------------------------

    class SbpReceiveDataWhileDeactivated : public SbpError
    {
    public:
        SbpReceiveDataWhileDeactivated() 
        : SbpError("SafeBiProto: receive data from transport while in deactivated state")
        {
        }
    };

    // -----------------------------------------------------------------

    class SbpRecvPackInvalidDataTypes : public SbpError
    {
    public:
        SbpRecvPackInvalidDataTypes() 
        : SbpError("SafeBiProto: receive packet with invalid data types")
        {
        }
    };

    // -----------------------------------------------------------------

    class SbpRecvPackInvalidType : public SbpError
    {
    public:
        SbpRecvPackInvalidType() 
        : SbpError("SafeBiProto: received packet with invalid packet type")
        {
        }
    };

    // -----------------------------------------------------------------

    class SbpRecvPackIsEmpty : public SbpError
    {
    public:
        SbpRecvPackIsEmpty() 
        : SbpError("SafeBiProto: received packet is empty")
        {
        }
    }; 
    
} // namespace SBProto

#endif

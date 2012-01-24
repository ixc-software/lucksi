#ifndef RTPERROR_H
#define RTPERROR_H

namespace iRtp
{

    class RtpError
    {
    public:

        enum ErrKind
        {
            // LwIP
            SocketIncomingPackSplit, // incoming datagram is to large
            SocketSendFail,

            // PC
            SocketPcError,
        };        

        RtpError(ErrKind kind, const std::string& desc) : m_kind(kind), m_desc(desc)
        {
        }

        ErrKind Kind() const { return m_kind; }
        const std::string& Desc() const { return m_desc; }

    private:

        const ErrKind m_kind; 
        const std::string m_desc;

    };

} // namespace iRtp

#endif

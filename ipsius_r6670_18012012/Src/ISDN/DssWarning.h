#ifndef DSSWARNING_H
#define DSSWARNING_H

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "Utils/ErrorsSubsystem.h"
#include "isdnpack.h"
#include "Utils/BaseSpecAllocated.h"
#include "DssCallParams.h"


namespace ISDN
{
    using std::string;
    using boost::shared_ptr;
    //class DssCallParams;

    // Базовый класс для всех сообщений об ошибках
    class DssWarning : public Utils::BaseSpecAllocated<IIsdnAlloc>
    {
    public:

        virtual string ToString() const = 0;

        virtual ~DssWarning() {}
    };

    // DssWarning in Call Context
    class CallWarning : public DssWarning  
    {
    protected:
        CallWarning(){}
    };

    // DssWarning in Stack context
    class StackWarning : public DssWarning 
    {
    protected:
        StackWarning(){}
    };

    // DssWarning in Stack context with IPacket
    class StackPacketWarning : public StackWarning
    {
    public:
        virtual QVector<byte> GetPacketContent()const = 0;
    };

    // DssWarning in Stack context - unexpected command from user
    class UnexpectedUserCmd : public StackWarning
    {
    protected:
        UnexpectedUserCmd(){}    
    };

    //-------------------------------------------------------------------------------

    class UnsupportedBroadcast : public StackWarning
    {    
        UnsupportedBroadcast(){}

    public:

        string ToString() const
        {
            return "Unsupported Broadcast";
        }

        static UnsupportedBroadcast* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) UnsupportedBroadcast;
        }
    };

    //-------------------------------------------------------------------------------

    class CantCreateIncommingCall : public StackWarning
    {
        CantCreateIncommingCall(){}

    public:        

        string ToString() const
        {
            return "Cant create incoming call";
        }

        static CantCreateIncommingCall* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) CantCreateIncommingCall;
        }
    };

    //-------------------------------------------------------------------------------

    //NeedRreleaseComplete
    class WrongCallRefInNoSetupPack : public StackWarning
    {
        string m_msg;

        WrongCallRefInNoSetupPack(const string& msg): m_msg(msg) {}

    public:        

        string ToString() const
        {            
            return "Wrong call reference in no Setup packet " + m_msg;         
        }

        static WrongCallRefInNoSetupPack* Create(IIsdnAlloc& alloc, const string& msg)
        {
            return new(alloc) WrongCallRefInNoSetupPack(msg);
        }
    };

    //-------------------------------------------------------------------------------

    class MaxRepeatFError : public StackWarning
    {        
        MaxRepeatFError(){}

    public:        

        string ToString() const
        {            
            return "Remote Layer2 is deafen. Cant receive UA in reestablish. Try restart L2.";   
        }

        static MaxRepeatFError* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) MaxRepeatFError;
        }
    };

    //-------------------------------------------------------------------------------

    class L2NotActve : public UnexpectedUserCmd
    {
        string m_msg;

        L2NotActve(const string& additionalInfo) : m_msg(additionalInfo){}

    public:               

        string ToString() const
        {
            return "L2 Not Active. " + m_msg;
        }

        static L2NotActve* Create(IIsdnAlloc& alloc, const string& additionalInfo)
        {
            return new(alloc) L2NotActve(additionalInfo);
        }
    };

    //-------------------------------------------------------------------------------

    class DssNotDeactivated : public UnexpectedUserCmd
    {
        string m_msg;

        DssNotDeactivated(const string& additionalInfo) : m_msg(additionalInfo){}

    public:               

        string ToString() const
        {
            return m_msg;
        }

        static DssNotDeactivated* Create(IIsdnAlloc& alloc, const string& additionalInfo)
        {
            return new(alloc) DssNotDeactivated(additionalInfo);
        }
    };    

    //-------------------------------------------------------------------------------

    class IeException : public StackWarning
    {       
        IeException(){}

    public:

        string ToString() const
        {
            return "Can`t create Ie from stream";
        }

        static IeException* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) IeException;
        }
    };

    class LinkToUserCallError : public StackWarning
    {
        shared_ptr<const DssCallParams> m_params;

        LinkToUserCallError(shared_ptr<const DssCallParams> params)
            : m_params(params)
        {}

    public:

        string ToString() const
        {
            std::stringstream ss;
            ss << "Binder to user-call turned to null in moment link connection procedure."
                << "\nProbably User-call was deleted." 
                << "\nCall parametrs info:\n" << m_params->ToString();
            return ss.str();
        }

        shared_ptr<const DssCallParams> GetRequestedCallParams()
        {
            return m_params;
        }

        static LinkToUserCallError* Create(IIsdnAlloc& alloc, shared_ptr<const DssCallParams> params)
        {
            return new(alloc) LinkToUserCallError(params);
        }
    };

    //-------------------------------------------------------------------------------

    class ShortPacket : public StackPacketWarning, boost::noncopyable
    {
        QVector<byte> m_l2data;
        
        ShortPacket(const QVector<byte> &l2data) : m_l2data(l2data) {}             

    public:
        
        QVector<byte> GetPacketContent() const // Override
        {
            return m_l2data;
        }

        string ToString() const // Override
        {
            return "Dss recive to short packet";
        }

        static ShortPacket* Create(IIsdnAlloc& alloc, const QVector<byte> &l2data)
        {
            return new(alloc) ShortPacket(l2data);
        }
    };

    //-------------------------------------------------------------------------------

    class PackWithUnknounIe : public StackPacketWarning, boost::noncopyable
    {
        QVector<byte> m_l2data;
        std::string m_ieInfo;

        PackWithUnknounIe(const QVector<byte> &l2data, const std::string& ieInfo) 
            : m_l2data(l2data),
            m_ieInfo(ieInfo)
        {
            m_ieInfo = "Dss recive packet with unknown/unsupported Ie: ";
            m_ieInfo += ieInfo;
        }             

    public:

        QVector<byte> GetPacketContent() const // Override
        {
            return m_l2data;
        }

        string ToString() const // Override
        {            
            return m_ieInfo;
        }

        static PackWithUnknounIe* Create(IIsdnAlloc& alloc, const QVector<byte> &l2data, const std::string& ieInfo)
        {
            return new(alloc) PackWithUnknounIe(l2data, ieInfo);
        }
    };

    // ------------------------------------------------------------------------------

    class PackWithUnknounMt : public StackPacketWarning, boost::noncopyable
    {
        QVector<byte> m_l2data;

        PackWithUnknounMt(const QVector<byte> &l2data) : m_l2data(l2data) {}             

    public:

        QVector<byte> GetPacketContent() const // Override
        {
            return m_l2data;
        }

        string ToString() const // Override
        {
            return "Dss recive packet with unknown/unsupported Mt";
        }

        static PackWithUnknounMt* Create(IIsdnAlloc& alloc, QVector<byte> l2data)
        {
            return new(alloc) PackWithUnknounMt(l2data);
        }
    };

    // ------------------------------------------------------------------------------

    class WrongPack : public StackPacketWarning, boost::noncopyable
    {
        QVector<byte> m_l2data;
        const std::string m_info;

        WrongPack(const QVector<byte> &l2data, const std::string& info) 
            : m_l2data(l2data), m_info(info) {}             

    public:

        QVector<byte> GetPacketContent() const // Override
        {
            return m_l2data;
        }

        string ToString() const // Override
        {
            return m_info;
        }

        static WrongPack* Create(IIsdnAlloc& alloc, const std::string& info, QVector<byte> l2data = QVector<byte>())
        {
            return new(alloc) WrongPack(l2data, info);
        }
    };

    class PackWithWrongCrvLen : public StackPacketWarning, boost::noncopyable
    {
        QVector<byte> m_l2data;

        PackWithWrongCrvLen(const QVector<byte> &l2data) : m_l2data(l2data) {}             

    public:

        QVector<byte> GetPacketContent() const // Override
        {
            return m_l2data;
        }

        string ToString() const // Override
        {
            return "Dss1 receive packet with wrong CallRef";
        }

        static PackWithWrongCrvLen* Create(IIsdnAlloc& alloc, QVector<byte> l2data)
        {
            return new(alloc) PackWithWrongCrvLen(l2data);
        }
    };


    //-------------------------------------------------------------------------------

    class PacketParseError : public StackPacketWarning, boost::noncopyable
    {        
        QVector<byte> m_l2data;
        
        PacketParseError(QVector<byte> l2data) : m_l2data(l2data) {}        

    public:        

        QVector<byte> GetPacketContent() const // Override
        {
            return m_l2data;
        }
    
        string ToString() const
        {
            return "Packet parsing failed";
        }

        static PacketParseError* Create(IIsdnAlloc& alloc, QVector<byte> l2data)
        {
            return new(alloc) PacketParseError(l2data);
        }
    };


    //-------------------------------------------------------------------------------

    class BadBChannelsAck : public CallWarning
    {
        BadBChannelsAck(){}

    public:

        string ToString() const
        {
            return "Other side response different count of B-channels by requested";
        }

        static BadBChannelsAck* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) BadBChannelsAck;
        }
    };

    //-------------------------------------------------------------------------------    

    class SetupConfirmErr : public CallWarning
    {
        SetupConfirmErr(){}

    public:

        string ToString() const
        {
            return "Error in \"setup confirmation\" procedure";
        }

        static SetupConfirmErr* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) SetupConfirmErr;
        }
    };

    //-------------------------------------------------------------------------------

    class ReleaseConfirmErr : public CallWarning
    {
        ReleaseConfirmErr(){}

    public:

        string ToString() const
        {
            return "Error in \"Release confirmation\" procedure";
        }

        static ReleaseConfirmErr* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) ReleaseConfirmErr;
        }
    };

    //-------------------------------------------------------------------------------

    class ConnectConfirmErr : public CallWarning
    {
        ConnectConfirmErr(){}

    public:

        string ToString() const
        {
            return "Error in \"Connect confirmation\" procedure";
        }

        static ConnectConfirmErr* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) ConnectConfirmErr;
        }
    };

    //-------------------------------------------------------------------------------

    class TimeoutIndication : public CallWarning
    {
        const std::string m_msg;

        TimeoutIndication(const std::string& msg) : m_msg(msg)
        {}

    public:

        string ToString() const
        {             
            return m_msg;
        }

        static TimeoutIndication* Create(IIsdnAlloc& alloc, const std::string& msg)
        {
            return new(alloc) TimeoutIndication(msg);
        }
    };

    //-------------------------------------------------------------------------------

    class UnexpectedPackReceived : public CallWarning
    {
        std::string m_msg;

        UnexpectedPackReceived(const std::string& packAsString)
            : m_msg("Unexpected Pack Received:\n\t" + packAsString)
        {}        

    public:

        string ToString() const
        {
            return m_msg;
        }        

        static UnexpectedPackReceived* Create(IIsdnAlloc& alloc, const std::string& packAsString)
        {
            return new(alloc) UnexpectedPackReceived(packAsString);
        }
    };

    //-------------------------------------------------------------------------------

    class TemporaryAnavailable : public CallWarning
    {        
        TemporaryAnavailable()            
        {}        

    public:

        string ToString() const
        {
            return "User cmd temporary unavailable";
        }        

        static TemporaryAnavailable* Create(IIsdnAlloc& alloc)
        {
            return new(alloc) TemporaryAnavailable;
        }
    };


    ESS_TYPEDEF_T(DssWarningExcept, boost::shared_ptr<const DssWarning>);


} // ISDN

#endif


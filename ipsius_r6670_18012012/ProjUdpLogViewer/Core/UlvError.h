
#ifndef __ULVError__
#define __ULVError__

#include "iNet/SocketError.h"

namespace Ulv
{
    // Base class for all LogView errors
    class Error
    {
    public:

        enum ErrType
        {
            TModelErr = 0,
            TUserErr = 1, 
            TCriticalErr = 2,
        };
        
        Error(const QString &err, ErrType type):
            m_err(err), m_type(type)
        {
        }

        Error(const std::string &err, ErrType type):
            m_err(QString::fromStdString(err)), m_type(type)
        {
        }

        std::string AsString() const { return m_err.toStdString(); }
        QString AsQString() const { return m_err; }
        ErrType Type() const { return m_type; }
        
    private:
        QString m_err;
        ErrType m_type;
    };

    // ---------------------------------------------------
    // Model errors

    class UdpSocketError : public Error
    {
    public:
        UdpSocketError(boost::shared_ptr<iNet::SocketError> err) : 
            Error(err->getErrorString(), TModelErr)
        {
        }
    };

    // ---------------------------------------------------

    class UdpSocketBindError : public Error
    {
    public:
        UdpSocketBindError() :
            Error(QString("UDP socket binding error"), TModelErr)
        {
        }
    };

    // ---------------------------------------------------
    // User errors

    /*class FileIOError : public Error
    {
        QString ErrText(const QString &fileName)
        {
            QString res = "File '";
            res += fileName;
            res += "' input/output error";

            return res;
        }
        
    public:
        FileIOError(const QString &fileName) : 
            Error(ErrText(fileName), TUserErr)
        {
        }
    };*/

    class FileIOError : public Error
    {
    public:
        FileIOError(const QString &err) :
            Error(err, TUserErr)
        {
        }
    };

    // ---------------------------------------------------

    class WStringBytesCountError : public Error
    {
    public:
        WStringBytesCountError() :
            Error(QString("Odd wstring bytes count"), TUserErr)
        {
        }
    };

    // ---------------------------------------------------

    class PackParsingError : public Error
    {
    public:
        PackParsingError() :
            Error(QString("UDP packet parsing error"), TUserErr)
        {
        }
    };

    // ---------------------------------------------------
    // Critical errors

    class CriticalError : public Error
    {
    public:
        CriticalError(const QString &err) :
            Error(err, TCriticalErr)
        {
        }
    };

    // ---------------------------------------------------


    
} // namespace Ulv

#endif

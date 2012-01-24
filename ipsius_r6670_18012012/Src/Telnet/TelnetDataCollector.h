#ifndef __DATACOLLECTOR__
#define __DATACOLLECTOR__

#include "ITelnet.h"
#include "TelnetHelpers.h"
#include "TelnetDataLogger.h"

namespace Telnet
{
    using Utils::SafeRef;

    // class for parsing socket input into Telnet protocol commands and user data
    class TelnetDataCollector : public boost::noncopyable
    {
        enum States
        {
            InsideCmd,          // inside cmd
            InsideSb,
            InsideText,         // inside ASCII text
        };

        // data
        SafeRef<ITelnetDataCollectorEvents> m_pOwner;
        States m_currentState;
        QByteArray m_textBuff;
        QByteArray m_pairBuff;
        QByteArray m_cmdBuff;
        // TelnetDataLogger &m_logger;

        // handlers 
        void ProcessChar(byte val);
        void ProcessIac(byte val);
        void ProcessCmd(byte val);
        void ProcessSb(byte val);
        void ProcessPair(byte val);
        void SendText(); 
        void ParseXoptSb(const QByteArray &data);
        QByteArray ExtractSbData();

        static byte At(const QByteArray &array, size_t index)
        {
            return TelnetHelpers::At(array, index);
        }

        static void Append(QByteArray &array, byte val)
        {
            TelnetHelpers::Append(array, val);
        }

    public:
        TelnetDataCollector(SafeRef<ITelnetDataCollectorEvents> pOwner/*, TelnetDataLogger &logger*/)
            : m_pOwner(pOwner), m_currentState(InsideText)/*, m_logger(logger)*/
        {}

        void ProcessSocketData(const QByteArray &data);
    };

} // namespace Telnet

#endif



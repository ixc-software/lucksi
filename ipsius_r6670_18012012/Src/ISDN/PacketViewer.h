#ifndef PACKETVIEWER_H
#define PACKETVIEWER_H

#include "stdafx.h"
#include "time.h"
#include "PacketInfo.h"
#include "isdnpack.h"
#include "Platform/Platform.h"
#include "IIsdnLog.h"
#include "Utils/QtHelpers.h"

namespace ISDN
{    

   

    class PacketViewer
    {            
        bool m_raw;
        ILoggable m_session;
        iLogW::LogRecordTag m_tagTeToNt;        
        iLogW::LogRecordTag m_tagNtToTe;

    public:

        PacketViewer(bool IsTEside, IIsdnLog& out, bool rawOn)
            : m_raw(rawOn),
            m_session(out, (IsTEside ? "PacketViewer-TE" : "PacketViewer-NT"), true),
            m_tagTeToNt( m_session.RegNewTag("TE --> NT ") ),
            m_tagNtToTe( m_session.RegNewTag("NT --> TE ") )                        
        {
        }

        PacketViewer(ILoggable& parentSession, bool rawOn)
            : m_raw(rawOn),
            m_session(parentSession, "/PacketViewer"),
            m_tagTeToNt( m_session.RegNewTag("TE --> NT ") ),
            m_tagNtToTe( m_session.RegNewTag("NT --> TE ") )
        {
        }

        void Draw(IsdnPacket* pPacket, bool fromTE)
        {
            if(!m_session.getTraceOn()) return;
            LogWriter log(&m_session, (fromTE ?  m_tagTeToNt : m_tagNtToTe) );
            
            //log.Write() << TimeStamp();
            PacketInfo pi(*pPacket, fromTE);
            if ( pi.BadPacket() )
            {
                log.Write() << "Warning! Bad packet!";
                return;
            }
            if (m_raw)
                log.Write() << "Raw: " << pi.AsBinary();
            log.Write() << "\n" << pi.CommonInfo();                
            
        }

        void SetLogActive(bool on)
        {
            m_session.setTraceOn(on);
        }
    
    };

} // namespace ISDN

#endif

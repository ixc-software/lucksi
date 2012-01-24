#include "stdafx.h"
#include "iLog/LogWrapperLibrary.h"

#include "Utils/BinaryReader.h"
#include "Utils/BinaryWriter.h"

#include "Utils/MemReaderStream.h"
#include "Utils/MemWriterStream.h"

#include "RtpCoreSession.h"
#include "RtpConstants.h"

namespace iRtp
{
    using Platform::byte;

    //-------------------------------------------------------------------------------------------    

    namespace
    {

        enum
        {
            CNoRtpHeaderOnSend = false,
        };

        enum 
        {
            msk_ProtVers = 0xc0,
            sh_ProtVers = 6,
            msk_PaddingBit = 0x20,
            msk_ExtBit = 0x10,
            msk_CSRCCount = 0x0f,
            msk_MarkerBit = 0x80,
            sh_MarkerBit = 7,
            msk_Payload = 0x7f
        };
       
        bool ValidateLenght(int length)
        {            
            return length > CRtpHeaderSize;
        } 

        word ReadWord(Utils::BinaryReader<Utils::MemReaderStream>& binaryReader)
        {
            byte b0 = binaryReader.ReadByte();
            byte b1 = binaryReader.ReadByte();
            return (static_cast<word>(b0) << 8)  | static_cast<word>(b1);
        }

        word ReadDWord(Utils::BinaryReader<Utils::MemReaderStream>& binaryReader)
        {
            dword d0 = static_cast<dword>(ReadWord(binaryReader));
            dword d1 = static_cast<dword>(ReadWord(binaryReader));
            return (d0 << 16) | d1;            
        }
        
        void WriteWord(Utils::BinaryWriter<Utils::MemWriterStream>& binaryWriter, word val)
        {
            binaryWriter.WriteByte(static_cast<byte>(val >> 8));
            binaryWriter.WriteByte(static_cast<byte>(val));
        }

        void WriteDWord(Utils::BinaryWriter<Utils::MemWriterStream>& binaryWriter, dword val)
        {
            WriteWord(binaryWriter, static_cast<word>(val >> 16));
            WriteWord(binaryWriter, static_cast<word>(val & 0xffff));
        }
        
    }; // namespace

    //-------------------------------------------------------------------------------------------

    struct RtpCoreSession::RtpHeader
        : public RtpHeaderForUser
    {
        RtpHeader()
            : RtpHeaderForUser(false, 0, CEmptyPayloadType)
        {}

        //bool m_extBit;
        int CountCsrc;        
        word Sequence;        
        dword Ssrc;
    };

    //-------------------------------------------------------------------------------------------
    
    bool RtpCoreSession::ParseAndConvertData( BidirBuffer* pBuff, RtpHeader& rezult )
    {
        int rawLenght = pBuff->Size();

        if (!ValidateLenght(rawLenght))
        {            
            if (m_log->LogActive(m_errorTag))
			{
                *m_log << m_errorTag << "Packet length smaller less standard header size. Length = " 
					<< rawLenght << iLogW::EndRecord;
			}
            return false;
        }

        Utils::MemReaderStream readerStream(pBuff->Front(), pBuff->Size());
        Utils::BinaryReader<Utils::MemReaderStream> buffReader(readerStream);

        //byte b = pBuff->At(0);
        byte b = buffReader.ReadByte();

        if ((b & msk_ProtVers) >> sh_ProtVers != CRtpVersion)
        {            
            if (m_log->LogActive(m_errorTag))
			{
				*m_log << m_errorTag << "Wrong protocol version" << iLogW::EndRecord;
			}
            return false;
        }

        // check if padding is present and remove it
        byte padding = 0;
        if (b & msk_PaddingBit) 
        {
            padding = 1;
            int skiped = *pBuff->Back();
            rawLenght -= skiped;
           
            if (!ValidateLenght(rawLenght))
            {            
				if (m_log->LogActive(m_errorTag))
				{
					*m_log << m_errorTag 
						<< "Packet length smaller less standard header size after skip padding. Length = " 
						<< rawLenght << iLogW::EndRecord;
				}
                return false;
            }
        }

        bool extBit = (b & msk_ExtBit) != 0; 
        rezult.CountCsrc = b & msk_CSRCCount;

        //b = pBuff->At(1);
        b = buffReader.ReadByte();
        rezult.Marker = (b & msk_MarkerBit) != 0;
        
        rezult.Payload = static_cast<RtpPayload>(b & msk_Payload);        
        

        rezult.Sequence = ReadWord(buffReader);//pBuff->ReadWordAt(2);                
        rezult.Timestamp = ReadDWord(buffReader);// pBuff->ReadDwordAt(4); 
        rezult.Ssrc = ReadDWord(buffReader);//pBuff->ReadDwordAt(8);                       

        // skip over header and any CSRC
        int skipAtFront = CRtpHeaderSize + (CRtpCsrcSize * rezult.CountCsrc);                                    

        // check if extension is present and skip it
        if (extBit) 
        {
            if (rawLenght - skipAtFront < CRtpExtensionHeaderSize)
            {                
				if (m_log->LogActive(m_errorTag))
				{
					*m_log << m_errorTag << "Extension bit present, but extension header not exist" 
						<< iLogW::EndRecord;
				}
                return false;
            }

            //skipAtFront += pBuff->ReadWordAt(2 + skipAtFront) * sizeof(dword) + CRtpExtensionHeaderSize;
            {//todo упростить
                int at = 2 + skipAtFront;
                Utils::MemReaderStream readerStream(&pBuff->At(at), pBuff->Size() - at);
                Utils::BinaryReader<Utils::MemReaderStream> buffReader(readerStream);
                skipAtFront += ReadWord(buffReader) * sizeof(dword) + CRtpExtensionHeaderSize;
            }            
        }

        // set property hasData
        //m_hasData = rawLenght - skipAtFront  > 0;

        // hide
        if (skipAtFront >= pBuff->Size())
        {
			if (m_log->LogActive(m_errorTag))
			{
				*m_log << m_errorTag << "Too many data for skip" << iLogW::EndRecord;
			}
            return false;
        }

        pBuff->AddSpaceFront(-skipAtFront);

        return true;
    }

    //-------------------------------------------------------------------------------------------

    void RtpCoreSession::AttachRtpFieldToBuffer(bool marker, dword ts, BidirBuffer* pUserData)
    {        
        ESS_ASSERT(m_txPayload != CEmptyPayloadType);

        pUserData->AddSpaceFront(CRtpHeaderSize); // Reserve header

        // return;  // debug
        
        Utils::MemWriterStream writerStream(pUserData->Front(), pUserData->Size());
        Utils::BinaryWriter<Utils::MemWriterStream> buffWriter(writerStream);
        
        // byte 0
        byte b = (CRtpVersion << sh_ProtVers);// & msk_ProtVers; //set version:
        buffWriter.WriteByte(b);                                       
         
        // byte 1
        b = (marker << sh_MarkerBit);// & sh_MarkerBit; //set marker        
        b |= m_txPayload; //set payload 
        buffWriter.WriteByte(b);                

        // byte 2,3
        WriteWord(buffWriter, getNextSeq()); //set seqNum    

        // byte 4-7
        WriteDWord(buffWriter, ts); //set timestamp

        // byte 8-15
        WriteDWord(buffWriter, m_ownerSsrc); //set SSRC
    }

    //-------------------------------------------------------------------------------------------

    word RtpCoreSession::getNextSeq( /*dword ssrc*/ )
    {
        // todo for multiple source find lastSeq by ssrc

        ++m_lastSeq;

        //m_lastSeq = (m_lastSeq + 1) & (m_seqMod - 1); , только если m_seqMod == 2^n

        return m_lastSeq;
    }

    //-------------------------------------------------------------------------------------------    

    void RtpCoreSession::RxData(SocketId id, BidirBuffer* pData, const HostInf& srcAddr)
    {        
        ESS_ASSERT( ValidateSocketId(id) );
        ESS_ASSERT(pData);         
        
		if (m_prof.TraceFull && m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "RxData" << iLogW::EndRecord;
		}

        RtpHeader header;
        bool parsingOk = ParseAndConvertData(pData, header);

       
        if ( parsingOk && m_seqCounters.ValidateSeqAndDoSizeStatistic(header.Sequence, header.Ssrc, pData->Size()) )
        {
            m_user.RxData(pData, header);            
        }
        else
            delete pData;
    }

    //---------------------------------------------------------------------

    void RtpCoreSession::ErrorInd(SocketId id, RtpError err)
    {
        ESS_ASSERT( ValidateSocketId(id) );
		if (m_log->LogActive(m_errorTag))
		{
			*m_log << m_errorTag << "Socket error" << iLogW::EndRecord;
		}
        ++m_socketErrLastStat;
        m_user.RtpErrorInd(err);
    }

    //---------------------------------------------------------------------
   
    void RtpCoreSession::TxData(bool marker, dword timestamp, BidirBuffer* pData)
    {
        ESS_ASSERT(pData != 0);
        ESS_ASSERT(m_destIsSet);

        if (!CNoRtpHeaderOnSend)
        {
            m_txLastStat.Add(pData->Size());

			if (m_log->LogActive(m_infoTag))
			{
				*m_log << m_infoTag << "TxData with ts = " << timestamp 
					<< ", marker = " << (marker ? "true" : "false") << iLogW::EndRecord;
			}
            AttachRtpFieldToBuffer(marker, timestamp, pData);      
        }

        m_sockets.RtpSock->TxData( pData );
    }

    //---------------------------------------------------------------------

    void RtpCoreSession::RtpSockDirectWrite(BidirBuffer* pData)
    {
        m_sockets.RtpSock->TxData(pData);
    }

    //---------------------------------------------------------------------

    void RtpCoreSession::TxEvent(dword timestamp, RtpEvent ev)
    {
        ESS_ASSERT(m_destIsSet);

		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "TxEvent with ts = "  << timestamp << iLogW::EndRecord;
		}

        ESS_HALT("Not implemented");
    }

    //---------------------------------------------------------------------

    RtpCoreSession::RtpCoreSession(const RtpParams& prof, 
		iLogW::ILogSessionCreator &logCreator,
        IRtpInfra& infra, 
        IRtpCoreToUser& user, 
        const HostInf &dstAddr ) : 
        m_prof(prof),
		m_log(logCreator.CreateSession("RtpCoreSession", prof.CoreTraceInd)),
		m_infoTag(m_log->RegisterRecordKindStr("Info")),
		m_errorTag(m_log->RegisterRecordKindStr("Error")),
        m_txPayload(CEmptyPayloadType),
        m_seqCounters(*this, prof),
        m_infra(infra),
        m_user(user),   
        m_lastSeq(0),
        m_socketErrLastStat(0),
        m_socketErrAllStat(0),
        m_sockets( infra.CreateSocketPair(*this, prof.UseRtcp) ),
        m_destIsSet(false)
    {
		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Created." << iLogW::EndRecord;
		}
        if (!dstAddr.Empty())
        {
            setDestAddr(dstAddr);            
        }        

        m_ownerSsrc = m_infra.GenRndSSRC();        
    }

	//---------------------------------------------------------------------

	RtpCoreSession::~RtpCoreSession()
	{
		if (m_log->LogActive(m_infoTag))
		{
			*m_log << m_infoTag << "Deleted." << iLogW::EndRecord;
		}
	}

    //---------------------------------------------------------------------

    void RtpCoreSession::setDestAddr(HostInf dstAddr)
    {
        if (m_destIsSet) ClearDstAddr();  // clear stats 
        m_destIsSet = true;

        ESS_ASSERT( m_sockets.RtpSock.get() );        
        m_sockets.RtpSock->SetDstAddress(dstAddr);
        if (m_sockets.RtcpSock.get() != 0)
        {
            dstAddr.Port(dstAddr.Port() + 1);
            m_sockets.RtcpSock->SetDstAddress(dstAddr);
        }
    }

    //---------------------------------------------------------------------

    void RtpCoreSession::setPayload( RtpPayload payloadType )
    {
        //ESS_ASSERT(m_txPayload == CEmptyPayloadType);
        m_txPayload = payloadType;
    }

    //---------------------------------------------------------------------

    void RtpCoreSession::SynchroCompleteWith( dword ssrc )
    {
        m_user.NewSsrcRegistred(ssrc);
    }

    bool RtpCoreSession::ValidateSocketId( SocketId id )
    {
        return ( id == m_sockets.RtpSock.get() /*|| id == m_sockets.RtcpSock.get()*/ );
        // uncomment if rtcp supported
    }
    
    
} // namespace iRtp


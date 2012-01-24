#ifndef RTPPAYLOAD_H
#define RTPPAYLOAD_H

namespace iRtp
{
    //rtp payload type specified in RFC 3551
    enum RtpPayload
    {
        CEmptyPayloadType = -1, 

        // audio
        PCMU    = 0,    // G.711 mu-law 
        GSM     = 3,
        G723    = 4,
        DVI4    = 5,    // clock rate  8000 Hz
        DVI4_16 = 6,    // clock rate 16000 Hz
        LPC     = 7, 
        PCMA    = 8,    // G.711 A-law
        G722    = 9,
        L16_2   = 10,   // clock rate 44100 Hz, channels 2  
        L16     = 11,   // clock rate 44100 Hz, channels 1  
        QCELP   = 12,
        CN      = 13,   // comfort noise specified in RFC 3389
        MPA     = 14,
        G728    = 15,
        DVI4_11 = 16,   // clock rate 11025 Hz
        DVI4_22 = 17,   // clock rate 22050 Hz
        G729    = 18,

        // video
        CelB = 25,
        JPEG = 26,
        H261 = 26,
        MPV  = 32, 
        MP2T = 33,
        H263 = 34,  

        // 96..127 may be defined dynamically 
        // through a conference control protocol

        CMaxPayloadType_Unused = 128
    };

	typedef std::vector<RtpPayload> RtpPayloadList;

} // namespace iRtp

#endif


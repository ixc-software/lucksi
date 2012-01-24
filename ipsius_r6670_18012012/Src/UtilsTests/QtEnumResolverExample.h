#ifndef __QTENUMRESOLVEREXAMPLE__
#define __QTENUMRESOLVEREXAMPLE__

#include "stdafx.h"

namespace UtilsTests
{

    class IeConst : public QObject
    {
        Q_OBJECT
        Q_ENUMS(CodingStandard TransCap TransMode TransRate)

    public:

        // 2 bits field
        enum CodingStandard
        {
            CCITT				 =  0x0,
            INTERNATIONAL        = 0x1,
            NATIONAL             = 0x2,
            NETWORK_SPECIFIC     = 0x3,
        };

        /* Transmit capabilities (5bit)*/
        enum TransCap
        {
            SPEECH				=	0x0,
            DIGITAL				=	0x08,
            RESTRICTED_DIGITAL	=	0x09,
            AUDIO_3_1K			=	0x10,
            //AUDIO_7K			=	0x11,	/* Depriciated ITU Q.931 (05/1998)*/
            DIGITAL_W_TONES		=	0x11,
            VIDEO				=   0x18
            // Other reserved
        };

        /*Transfer Mode 2bits (bit 7,6 in Bearer)*/
        enum TransMode
        {
            Circuit = 0,
            Packet = 2
            // Other reserved
        };

        /*Information Transfer Rate  (5 low bits in Bearer)*/
        enum TransRate
        {
            InPacket = 0,
            KbPs64 = 0x10,
            KbPs128 = 0x11,
            KbPs384 = 0x13,
            KbPs1536 = 0x15,
            KbPs1920 = 0x17,
            MultiRate = 0x18
            // Other reserved
        };

    };

}  // namespace UtilsTests

#endif


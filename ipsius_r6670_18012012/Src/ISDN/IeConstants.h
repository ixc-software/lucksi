/**************Standard Q931 constants value using in Ie******************/

#ifndef IECONSTANTS_H
#define IECONSTANTS_H

#include "Utils/ErrorsSubsystem.h"

namespace ISDN
{
    class IeConstants : public QObject
    {

        Q_OBJECT
        Q_ENUMS(SideType Location ProgressDscr CodingStandard TransCap TransMode TransRate
            IntfType PrefExcl ChanSelect NumOrMap ChanType TypeOfNum NumPlan PresentInd 
            ScreeningInd UserInfo SignalValue RestartClass)

        IeConstants();

    public:
		enum SideType 
		{ 
			NT, 
			TE
		};
		Q_ENUMS(SideType);

        enum Location //Q.980 Location field for IeCause (bits 4-1), IeProgressInd
        {
            U = 0,   //user
            LPN = 1, //local private network
            LN = 2,  //public network serving the local user
            TN = 3,  //transit network
            RLN = 4, //public network serving the local user 
            RPN = 5, //private network serving the remote user 
            INTL = 7,//international network 
            BI = 10,  //network beyond interworking point 

            SpecNonStandart = 11
            //All other reserved
        };    

        enum ProgressDscr
        {
            NotEndToEndISDN = 1, // Call is not end-to-end
            DestAdrNonIsdn = 2,
            OrigAdrNonIsdn = 3,
            HasRetToIsdn = 4,
            Interworking = 5,
            InBand = 8,
            // All other reserved            
        };
   
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

        // bit - 5 in octet - 2 Ie: ChanId
        enum IntfType
        {
            Bri = 0,
            Other = 1
        };

        // bit - 3 in octet - 2 Ie: ChanId
        enum PrefExcl
        {
            Preferred = 0, // желательные каналы
            Exclusive = 1  // только указанные
        };

        // bits - 0-1 in octet - 2 Ie: ChanId
        enum ChanSelect
        {
            NoChannel = 0,
            B1orIndInFollowing = 1,
            B2orReserved = 2,
            AnyChannel = 3
        };

        // bit - 4 in octet - 3.2 Ie: ChanId
        enum NumOrMap
        {
            Num = 0,
            Map = 1
        };

        // bits - 0-3 in octet - 3.2 Ie: ChanId
        enum ChanType
        {
            BChannels = 0x3,
            H0 = 0x6,
            H11 = 0x8,
            H12 = 0x9
            /// other reserved
        };

        // bits - 4-6 in octet - 3 Ie: Called/CallingNumber
        enum TypeOfNum
        {
            UnknownType = 0,
            International = 1,
            National = 2,
            NetSpec = 3,
            Subscriber = 4,
            // value 5 - reserved
            Abbreviated = 6,
            Ext = 7
        };

        // bits - 0-4 in octet - 3 Ie: Called/CallingNumber
        enum NumPlan
        {
            UnknownPlan = 0,
            ISDNtelephony = 1, //
            Data = 3,
            Telex = 4,
            NationalPlan = 8,
            Private = 9,
            ExtReserve = 15
            // all other reserved
        };

        // bits - 6-7 in octet - 3a Ie: CallingNumber
        enum PresentInd
        {
            Allowed = 0, //
            Restricted = 1,
            NotAvailable = 2,
            PesenIndReserved = 3
        };

        // bits - 0-1 in octet - 3a Ie: CallingNumber
        enum ScreeningInd
        {       
            UserProv_NotScreened = 0, //
            UserProv_Passed = 1,
            UserProv_Failed = 2,
            NetProv = 3
        };

        enum CauseNum
        {
            UnallocatedNumber = 1,
			NormCallClearing = 16,
			UserBusy = 17,
			NoUserResponding = 18,
            CallRejected = 21,
			NumberChanged = 22,
			ExchangeRoutingError = 25,
            DestOutOfOrder = 27,
			InvalidNumber = 28,
            NormUnspec = 31,
            NoCircChanAvailable = 34,
			NetworkOutOfOrder = 38,
            TemporaryFailure = 41, 
            NoRequestedCircChanAvailable = 44,
            ResUnavailableUspec = 47,            
			ServiceUnavailable = 63,
			ServiceNotImplemented = 79,
            MndIeIsMissing = 96,
            InvalidIeContent = 100,
            RecoveryTimerExpire = 102,
			Interworking_127 = 127,
        };


        // bits 0-4 in octet 5 
        enum UserInfo //PayloadType
        {
            V110_X30 = 1,
            G711_PCMU = 2,
            G711_PCMA = 3,
            G721_ADPCM = 4,
            H2xx = 5,
            Non_CCITT = 7,
            V120 = 8,
            X31_HDLC = 9,

            UserInfoMax = 10, // non standard
            /*All other reserved*/
        };

        // single byte value
        enum SignalValue
        {
            DialTone = 0,   
            RingBackToneOn = 1,
            InterceptToneOn = 2,
            NetCongestionToneOn = 3, 
            BusyToneOn = 4,
            ConfirmToneOn = 5,
            AnswToneOn = 6,
            CallWaitingToneOn = 7,
            OffHookWarning = 8,
            PreemptionToneOn = 9,

            TonesOff = 63,
            AllertingOn_p0 = 64,
            AllertingOn_p1 = 65,
            AllertingOn_p2 = 66,
            AllertingOn_p3 = 67,
            AllertingOn_p4 = 68,
            AllertingOn_p5 = 69,
            AllertingOn_p6 = 70,
            AllertingOn_p7 = 71,

            AllertingOff = 79,

            // All other reserved
        };

        // 0-2 bits in 2 octet
        enum RestartClass
        {
            Channels = 0,
            SingleIntf = 6,
            AllIntf = 7
        };

    };

    //!!!! Должно вычислятся "как бинарное представление коэфицента базовой скорости"
    // в диапазоне [2, колличество B-каналов]
    //enum Multiplier
    //{
    //    Mult = 0x23, /*A-Law Audio*/         
    //};  

        
} // namespace ISDN

#endif


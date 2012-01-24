#ifndef RTPEVENT_H
#define RTPEVENT_H

namespace iRtp
{
    
    // correspond to RFC 2833
    enum RtpEvent
    {
        CEmptyEventType = -1,

        // DTMF Events
        // 0-9, *, #, A-D, Flash  = 0..16
        Dtmf0     = 0,
        Dtmf1     = 1,
        Dtmf2     = 2,
        Dtmf3     = 3,
        Dtmf4     = 4,
        Dtmf5     = 5,
        Dtmf6     = 6,
        Dtmf7     = 7,
        Dtmf8     = 8,
        Dtmf9     = 9,
        DtmfStar  = 10,    // *
        DtmfUnset = 11,    // #
        DtmfA     = 12,    
        DtmfB     = 13,    
        DtmfC     = 14,     
        DtmfD     = 15,
        DtmfFlash = 16,    // Flash

        // Data Modem and Fax Events
        ANS    = 32,
        ANS_   = 33,    // /ANS
        ANSam  = 34, 
        ANSam_ = 35,    // /ANSam  
        CNG    = 36,
        V21Channel1_0 = 37,  // V.21 channel 1, "0" bit
        V21Channel1_1 = 38,  // V.21 channel 1, "1" bit
        V21Channel2_0 = 39,  // V.21 channel 2, "0" bit
        V21Channel2_1 = 40,  // V.21 channel 2, "1" bit
        CRdi   = 41,
        CRdr   = 42,
        CRe    = 43,
        ESi    = 44,
        ESr    = 45,
        MRdi   = 46,
        MRdr   = 47,
        MRe    = 48,
        CT     = 49,

        // Line Events
        OffHook                 = 64,
        OnHook                  = 65,
        DialTone                = 66,
        PABXinternalDialTone    = 67,
        SpecialDialTone         = 68,
        SecondDialTone          = 69,
        RingingTone             = 70,
        SpecialRingingTone      = 71,
        BusyTone                = 72,
        CongestionTone          = 73,
        SpecialInformationTone  = 74,
        ComfortTone             = 75,
        HoldTone                = 76,
        RecordTone              = 77,
        CallerWaitingTone       = 78,
        CallWaitingTone         = 79,
        PayTone                 = 80,
        PositiveIndicationTone  = 81,
        NegativeIndicationTone  = 82,
        WarningTone             = 83,
        IntrusionTone           = 84,
        CallingCardServiceTone  = 85,
        PayphoneRecognitionTone = 86,
        CPEAlertingSignal       = 87,
        OffHookWarningTone      = 88,
        Ring                    = 89,

        // Country-specific Line events
        AcceptanceTone           = 96,
        ConfirmationTone         = 97,
        DialToneOrRecall         = 98,   
        EndOfThreePartyServiceTone = 99,
        FacilitiesTone           = 100,
        LineLockoutTone          = 101,
        NumberUnobtainableTone   = 102,
        OfferingTone             = 103,
        PermanentSignalTone      = 104,
        PreemptionTone           = 105,
        QueueTone                = 106,
        RefusalTone              = 107,
        RouteTone                = 108,
        ValidTone                = 109,
        WaitingTone              = 110,
        EndOfPeriod              = 111,  // Warning tone
        PIPTone                  = 112   // Warning Tone 

        // Trunk events
        // 128..173
    };

} // namespace iRtp

#endif

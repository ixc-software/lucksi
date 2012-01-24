/*
/ Descriptions all supported types of DSS1 Information Element 
/ with standard(q931) protocol discriminator and codeset 
/ and registrations it in a IeFactory
*/


#ifndef IETYPES_H
#define IETYPES_H

#include "stdafx.h"

#include "IeFactory.h"
#include "IeConstants.h"
#include "IsdnUtils.h"
#include "Utils/InitVar.h"
#include "baseie.h"

namespace ISDN
{
    using boost::shared_ptr;
    
    class DssCause;


    //-----all type ie`s-----------

    using Utils::InitProp;
    using Utils::InitVar;
    using IsdnUtils::CorrectSet;

    class BearerCapability : public BaseIe< 0x04, 0 >
    {
        typedef BaseIe< 0x04, 0 > Base;
        InitProp<IeConstants::CodingStandard> m_codStd;
        InitProp<IeConstants::TransCap> m_transCap;
        InitProp<IeConstants::TransMode> m_transMode;
        InitProp<IeConstants::TransRate> m_transRate;
        InitProp<int> m_multiplier; // Multiplier
        InitProp<IeConstants::UserInfo> m_userPayload;
        // Other field (24 units)       

    public:

        BearerCapability(const RawData& raw, IsdnInfra& infra);

        BearerCapability(            
            IeConstants::CodingStandard codStd = IeConstants::CCITT,
            IeConstants::TransCap transCap = IeConstants::AUDIO_3_1K,            
            IeConstants::TransMode transMode = IeConstants::Circuit,
            IeConstants::TransRate transRate = IeConstants::KbPs64, int rateMultiplier = 23,
            IeConstants::UserInfo payload = IeConstants::G711_PCMA);        

        IeConstants::TransCap GetTransCap();

        InitProp<IeConstants::UserInfo> GetUserInfo();
       
    private:
        void WriteContent(std::stringstream& out) const;
       
        // BaseIe Impl
        void Serialize(std::vector<byte>& raw) const;                 
    };


    //-----------------------------

    class IeCallState : public BaseIe<0x14, 0>
    {
        typedef BaseIe<0x14, 0> Base;

        InitProp<IeConstants::CodingStandard> m_codStd;
        InitProp<int> m_stateValue;

    public:

        IeCallState(const RawData& raw, IsdnInfra& infra);

        IeCallState(int stateValue, IeConstants::CodingStandard codStd = IeConstants::CCITT);

        IeConstants::CodingStandard GetCodStd();

        int GetStateValue();

    private:

        void WriteContent(std::stringstream& out) const;

        void Serialize(std::vector<byte>& raw) const;             
        
    };

    //------------------------------------------------------------------------

    //FF declare
    //class BChannelsWrapper;

    class IeChannelIdentification : public BaseIe<0x18, 0>
    {
        typedef BaseIe<0x18, 0> Base;

        InitProp<bool> m_IntIdPresent; // интерфес по умолчанию
        InitProp<IeConstants::IntfType> m_intType; // no Bri (Pri)
        InitProp<IeConstants::PrefExcl> m_prefExcl; // Pref
        InitProp<bool> m_IsDchanInd; //false
        InitProp<IeConstants::ChanSelect> m_chanSelect; // IndInFollowing

        InitProp<int> m_IntfId; // if m_IntIdPresent

        InitProp<IeConstants::CodingStandard> m_codStd;
        InitProp<IeConstants::NumOrMap> m_numOrMap;
        InitProp<IeConstants::ChanType> m_chanType;

        SetBCannels m_setB;

        
    public:

        IeChannelIdentification(const RawData& raw, IsdnInfra& infra);

        IeChannelIdentification(const SetBCannels& setChannels, InitProp<int> intfId, IeConstants::PrefExcl prefExcl = IeConstants::Preferred);

        /*specific methods:*/
        const SetBCannels& GetBchannels() const;

        IeConstants::ChanSelect GetChanSelection() const;

        IeConstants::PrefExcl GetPrefExcl() const;

        InitProp<int> GetIntfId() const {return m_IntfId;}

    private:

        void WriteContent(std::stringstream& out) const;

        void Serialize(std::vector<byte>& raw) const;         
    };

    //------------------------------------------------------------------------

    class IeCalledPartyNumber: public BaseIe<0x70, 0>
    {        
        typedef BaseIe<0x70, 0> Base;

        DssPhoneNumber m_num;

    public:

        IeCalledPartyNumber(const RawData& raw, IsdnInfra& infra);

        IeCalledPartyNumber(const DssPhoneNumber& num);

        DssPhoneNumber GetNum();       

    private:

        void WriteContent(std::stringstream& out) const;
       
        void Serialize(std::vector<byte>& raw) const;             
    };

    //------------------------------------------------------------------------
    
    class IeCallingPartyNumber : public BaseIe<0x6c, 0>
    {   
        typedef BaseIe<0x6c, 0> Base;

        // Optional 3a octet
        InitProp<IeConstants::PresentInd> m_prInd;
        InitProp<IeConstants::ScreeningInd> m_screenInd;

        DssPhoneNumber m_num;
       
    public:

        IeCallingPartyNumber(const RawData& raw, IsdnInfra& infra);

        IeCallingPartyNumber(const DssPhoneNumber& num);                

        IeConstants::PresentInd GetPresentInd();

        DssPhoneNumber GetNum();

        IeConstants::ScreeningInd GetScreeningInd();
        
    private:

        void WriteContent(std::stringstream& out) const;

        void Serialize(std::vector<byte>& raw) const;                       
    };

    //------------------------------------------------------------------------

    class SendingComplete : public BaseIe<0xA1, 0> 
    {

        typedef BaseIe<0xA1, 0> Base;

    public:

        SendingComplete(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
        {
            if ( (raw[0] & 0x7f) != GetId().Id() )
                ESS_ASSERT("Invalid IeId in raw");
        }

        SendingComplete() : Base(writeOnly){}
        
    private:

        void WriteContent(std::stringstream& out) const
        {
            out << "No Content (single-byte Ie)";
        }

        void Serialize(std::vector<byte>& raw) const       
        {            
            ESS_ASSERT( raw.size()==0 );
            int ext = 1 << 7;
            raw.push_back( ext | GetId().Id() );            
        }
       
    };

    //------------------------------------------------------------------------

    class IeCause : public BaseIe<0x08, 0>
    {   
        typedef BaseIe<0x08, 0> Base;

        InitProp<IeConstants::CauseNum> m_causeNum;
        InitProp<IeConstants::CodingStandard> m_codStd;
        InitProp<IeConstants::Location> m_location; 
        InitProp<Platform::byte> m_dignosticValue;

        IsdnInfra* m_pInfra;       

    public:

        IeCause(const RawData& raw, IsdnInfra& infra);

        IeCause( shared_ptr<const DssCause> pCause );

        /*int GetCauseNum() const { return m_causeNum; }
        CodingStandard GetCodStd() const { return m_codStd; }
        Location GetLocation() const { return m_location; }*/

        shared_ptr<const DssCause> GetCause();
        
    private:

        void WriteContent(std::stringstream& out) const;

        void Serialize(std::vector<byte>& raw) const;         
       
    };

    //------------------------------------------------------------------------

    class IeProgressInd : public BaseIe<0x1E, 0>
    {  
        typedef BaseIe<0x1E, 0> Base;
       
        InitProp<IeConstants::CodingStandard> m_codStd;
        InitProp<IeConstants::Location> m_location; 
        IeConstants::ProgressDscr m_descr;
        

    public:

        IeProgressInd(const RawData& raw, IsdnInfra& infra);

        IeProgressInd( IeConstants::Location loc, IeConstants::ProgressDscr descr,
            IeConstants::CodingStandard codStd = IeConstants::CCITT);

        IeConstants::ProgressDscr getDescr() const;
        
    private:

        void WriteContent(std::stringstream& out) const;

        void Serialize(std::vector<byte>& raw) const;         

    };

    //------------------------------------------------------------------------

    //TODO пока не обрабатывается,
    class IeDisplay : public BaseIe<0x28, 0>
    {
        typedef BaseIe<0x28, 0> Base;

    public:
        IeDisplay(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
        {            
            //AssertSizeAndId(raw, 82, pInfra);//max len 34 or 82 TODO
        }

        //IeDisplay(....) - нет тк я эту Ie не формирую

    private:

        void WriteContent(std::stringstream& out) const
        {
            out << "Parsing IeDisplay not realized. It`s Ie empty!.";
        }

        void Serialize(std::vector<byte>& raw) const
        {
            ESS_ASSERT(0 && "Try using not realized IeDisplay for packet cration");            
        }
    };

    //------------------------------------------------------------------------

    class IeSignal
        : public BaseIe<0x34, 0>
    {
        typedef BaseIe<0x34, 0> Base;

        InitProp<IeConstants::SignalValue> m_signalValue;

    public:

        IeSignal(const RawData& raw, IsdnInfra& infra);

        IeSignal(IeConstants::SignalValue val);

    private:

        void WriteContent(std::stringstream& out) const;
        
        void Serialize(std::vector<byte>& raw) const;         
    };

    //------------------------------------------------------------------------

    class IeRestartInd
        : public BaseIe<0x79, 0>
    {
        typedef BaseIe<0x79, 0> Base;

        IeConstants::RestartClass m_val;

    public:

        IeRestartInd(const RawData& raw, IsdnInfra& infra);
        IeRestartInd(IeConstants::RestartClass val);
        IeConstants::RestartClass getValue() const;

    private:

        void WriteContent(std::stringstream& out) const;        
        void Serialize(std::vector<byte>& raw) const;         
    };

    //------------------------------------------------------------------------

    // Специальный тип Ie создаваемый фабрикой если приходит IeId.
    // Не может быть созданна как исходящая. Не требует регистрации в фабрике(правильно ли?).
    class UnknownIeType 
        : public BaseIe<IeFactory::CSpecId_IeUnknown, IeFactory::CSpecCodSet>
    {
        typedef BaseIe<IeFactory::CSpecId_IeUnknown, IeFactory::CSpecCodSet> Base;
        std::vector<byte> m_dump;
        IeId m_id;

    public:
        UnknownIeType(IeId id, const RawData& raw, IsdnInfra& infra);

        void WriteContent(std::stringstream& out) const;
    private:        
        void Serialize(std::vector<byte>& raw) const;
    };

    //------------------------------------------------------------------------


    static void RegIes(IeFactory& factory)
    {
        factory.RegisterIe<BearerCapability>();
        factory.RegisterIe<IeCallState>();
        factory.RegisterIe<IeChannelIdentification>();
        factory.RegisterIe<IeCalledPartyNumber>();
        factory.RegisterIe<IeCallingPartyNumber>();
        factory.RegisterIe<SendingComplete>();
        factory.RegisterIe<IeCause>();
        factory.RegisterIe<IeProgressInd>();
        factory.RegisterIe<IeRestartInd>();

        //Dummy Ie:
        factory.RegisterIe<IeDisplay>();
        //....
    }

} // namespace ISDN

#endif


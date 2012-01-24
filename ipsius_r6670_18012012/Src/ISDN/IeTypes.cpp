#include "stdafx.h"
#include "Utils/FullEnumResolver.h"
#include "Utils/ErrorsSubsystem.h"

#include "IeTypes.h"
#include "DssWarning.h"
#include "SetBCannels.h"
#include "DssCause.h"

namespace 
{
    const bool CAddOctotorp = false;

    void WriteField(const std::string& caption, const std::string& val, std::stringstream& out)
    {
        out << caption << ": " << val << "; ";
    }
    
    template<class TEnum>
    void WriteField(const std::string& caption, Utils::InitProp<TEnum> val, std::stringstream& out)    
    {
        if (!val.IsInited()) return;
        WriteField(caption, Utils::EnumResolve<TEnum>(val), out);        
    }
    
    
    
    
    // InitProp<bool> spec
    void WriteField(const std::string& caption, Utils::InitProp<bool> val, std::stringstream& out)    
    {
        if (!val.IsInited()) return;
        WriteField(caption, val ? "true" : "false", out);        
    }    
    
    // InitProp<int> spec
    void WriteField(const std::string& caption, Utils::InitProp<int> val, std::stringstream& out)    
    {
        if (!val.IsInited()) return;
        WriteField(caption, Utils::IntToString(val), out);        
    }

    // InitProp<byte> spec
    void WriteField(const std::string& caption, Utils::InitProp<Platform::byte> val, std::stringstream& out)    
    {
        if (!val.IsInited()) return;
        WriteField(caption, Utils::IntToString(val), out);        
    }

} // namespace 


namespace ISDN
{       

    BearerCapability::BearerCapability(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
    {        
        AssertSizeAndId(raw, 4, infra);
        const int len = raw[1];        

        byte b;            

        CorrectSet converter;

        m_codStd.Init( IeConstants::CodingStandard( ExtractValue(raw[2], 5, 6) ) );            

        IeConstants::TransCap transCap;
        converter.Add(IeConstants::AUDIO_3_1K);
        converter.Add(IeConstants::DIGITAL);
        converter.Add(IeConstants::DIGITAL_W_TONES);
        converter.Add(IeConstants::RESTRICTED_DIGITAL);
        converter.Add(IeConstants::SPEECH);
        converter.Add(IeConstants::VIDEO);            
        b = ExtractValue(raw[2], 0, 4);
        if ( !converter.TryConvert(b, transCap) )
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        converter.Reset();
        m_transCap.Init(transCap);

        IeConstants::TransMode transMode;
        converter.Add(IeConstants::Circuit);
        converter.Add(IeConstants::Packet);
        b = ExtractValue(raw[3], 5, 6);
        if ( !converter.TryConvert(b, transMode) )
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        converter.Reset();
        m_transMode.Init(transMode);

        IeConstants::TransRate transRate;
        converter.Add(IeConstants::InPacket);
        converter.Add(IeConstants::KbPs128);
        converter.Add(IeConstants::KbPs1536);
        converter.Add(IeConstants::KbPs1920);
        converter.Add(IeConstants::KbPs384);
        converter.Add(IeConstants::KbPs64);
        converter.Add(IeConstants::MultiRate);
        b = ExtractValue(raw[3], 0, 4);
        if ( !converter.TryConvert(b, transRate) )
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        converter.Reset();
        m_transRate.Init(transRate);           

        int currPos = 4;        

        if (m_transMode == IeConstants::Circuit && m_transRate == IeConstants::MultiRate) // next octet interpret as m_multiplier
        {
            if (currPos - 2 >= len) // метка длины или реальная длина не позволяет прочесть след элемент
                ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
            
            b = ExtractValue(raw[currPos++], 0, 6);
            m_multiplier.Init(b);
        }          
        
        if (currPos - 2 < len) // если содержится след элемент
        {            
            IeConstants::UserInfo userInfo;
            converter.Add(IeConstants::V110_X30);
            converter.Add(IeConstants::G711_PCMU);
            converter.Add(IeConstants::G711_PCMA);
            converter.Add(IeConstants::G721_ADPCM);
            converter.Add(IeConstants::H2xx);
            converter.Add(IeConstants::Non_CCITT);
            converter.Add(IeConstants::V120);
            converter.Add(IeConstants::X31_HDLC);

            b = ExtractValue(raw[currPos++], 0, 4);

            if ( !converter.TryConvert(b, userInfo))
                ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));

            converter.Reset();

            m_userPayload.Init(userInfo);
        }

    }

    BearerCapability::BearerCapability(        
        IeConstants::CodingStandard codStd,
        IeConstants::TransCap transCap,
        IeConstants::TransMode transMode,
        IeConstants::TransRate transRate , int rateMultiplier,
        IeConstants::UserInfo payload
        ) : Base(writeOnly)           
    {        
        if ( (transMode == IeConstants::Circuit && transRate == IeConstants::InPacket) || 
            (transMode == IeConstants::Packet && (transRate != IeConstants::InPacket && 
            transRate != IeConstants::MultiRate) ) ) // MultiRate/64???
            ESS_ASSERT(0 && "Invalid combination arg");

        m_codStd.Init(codStd);
        m_transCap.Init(transCap);
        m_transMode.Init(transMode);
        m_transRate.Init(transRate);

        if (transMode == IeConstants::Circuit && transRate == IeConstants::MultiRate) // use multiplier
        {
            m_multiplier.Init(rateMultiplier);
        }       

        m_userPayload.Init(payload);
    }

   

    InitProp<IeConstants::UserInfo> BearerCapability::GetUserInfo()
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_userPayload;
    }

    IeConstants::TransCap BearerCapability::GetTransCap()
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_transCap;
    }

    void BearerCapability::WriteContent(std::stringstream& out) const
    {
        ESS_ASSERT(Mode() == readOnly);
        WriteField("CodingStandard", m_codStd, out);
        WriteField("TransCapability", m_transCap, out);
        WriteField("TransMode", m_transMode, out);
        WriteField("TransRate", m_transRate, out);
        WriteField("Multiplier", m_multiplier, out);
        WriteField("UserInfo", m_userPayload, out);
    }

    void BearerCapability::Serialize(std::vector<byte>& raw) const  // override
    {       
        raw.push_back( GetId().Id() ); // Set identifier
        raw.push_back(0); // reserved len-field

        byte b, ext;

        // Обязательные поля
        ext = 1 << 7;
        b = ext | ((m_codStd << 5) | m_transCap); // octet 3
        raw.push_back(b);

        ext = 1 << 7;
        b = ext | ((m_transMode << 5) | m_transRate);  // octet 4    
        raw.push_back(b);

        // Необязательные поля
        if(m_multiplier.IsInited())
        {
            ext = 1 << 7;
            b = ext | m_multiplier; // octet 5
            raw.push_back(b);
        }

        // payload
        const int layerIdent = 1;
        ext = 1 << 7;
        b = ext | ((layerIdent << 5) | m_userPayload);
        raw.push_back(b);
        
        // Other fields ...

        FixLen(); // Записываем длину Ie во 2 октет        
    }

    IeCallState::IeCallState(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
    {
        AssertSizeAndId(raw, 3, infra);
        byte b;
        m_codStd.Init( IeConstants::CodingStandard(ExtractValue(raw[2], 6, 7)) );
        m_stateValue.Init( ExtractValue(raw[2], 0, 5) );        
    }

    IeCallState::IeCallState(int stateValue, IeConstants::CodingStandard codStd) : Base(writeOnly)
    {
        ESS_ASSERT(stateValue >= 0 && stateValue <=63);
        m_codStd.Init(codStd);
        m_stateValue.Init(stateValue);        
    }

    IeConstants::CodingStandard IeCallState::GetCodStd()
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_codStd;
    }

    int IeCallState::GetStateValue()
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_stateValue;
    }

    void IeCallState::WriteContent(std::stringstream& out) const
    {            
        ESS_ASSERT(Mode() == readOnly);
        WriteField("CodingStandard", m_codStd, out);
        WriteField("StateValue", m_stateValue, out);
    }

    void IeCallState::Serialize(std::vector<byte>& raw) const // override
    {        
        raw.push_back( GetId().Id() ); // Set identifier
        raw.push_back(0); // reserved len-field

        byte b;

        // Обязательные поля            
        b = (m_codStd << 6) | m_stateValue;
        raw.push_back(b);

        FixLen();
    }

    IeChannelIdentification::IeChannelIdentification(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
    {        
        AssertSizeAndId(raw, 3, infra);

        byte b = raw[2];
        m_IntIdPresent.Init( ExtractValue(b, 6, 6) );
        m_intType.Init( IeConstants::IntfType(ExtractValue(b, 5, 5)) );
        m_prefExcl.Init( IeConstants::PrefExcl(ExtractValue(b, 3, 3)) );
        m_IsDchanInd.Init( ExtractValue(b, 2, 2) );
        m_chanSelect.Init( IeConstants::ChanSelect(ExtractValue(b, 0, 1)) );

        int intfIdentLen = 0;        
        if(m_IntIdPresent.get())
        {
            //"Temporary IntId field not supported in CanId Ie"
            //ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));       
            b = raw[3];
            if (ExtractValue(b, 7, 7) == 0) // multi id not supported
                ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
            m_IntfId = ExtractValue(b, 0, 6);
            intfIdentLen = 1;
        }

        if (m_intType != IeConstants::Other) return; // дальнейший разбор только если не BRI иначе разбо закончен        
       
        AssertSizeAndId(raw, 4, infra);/*TODO 4 or 5??*/
        b = raw[3 + intfIdentLen];
        m_codStd.Init( IeConstants::CodingStandard(ExtractValue(b, 5, 6)) );
        m_numOrMap.Init( IeConstants::NumOrMap(ExtractValue(b, 4, 4)) );

        CorrectSet converter;
        converter.Add(IeConstants::BChannels);
        converter.Add(IeConstants::H0);
        converter.Add(IeConstants::H11);
        converter.Add(IeConstants::H12);
        IeConstants::ChanType chanType;
        if (!converter.TryConvert( ExtractValue(b, 0, 3), chanType ))
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));

        m_chanType.Init( chanType );

        if (chanType != IeConstants::BChannels)     
            //TODO Other type
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));     

       
        if(m_numOrMap != IeConstants::Num) 
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        
        // read as SetBCannels                
        //int currPos = 3 + intfIdentLen;
        int len = raw[1];
        AssertSizeAndId(raw, 2+len, infra);
        for (int i = 4 + intfIdentLen; i < 2 + len; ++i )
        {
            int num = raw[i] & 0x7f;
            if (num >= m_setB.Capacity())
                ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
            m_setB.Set(raw[i] & 0x7f);
        }        
    }

    IeChannelIdentification::IeChannelIdentification(const SetBCannels& setChannels,
        InitProp<int> intfId,
        IeConstants::PrefExcl prefExcl) 
        : Base(writeOnly),
        m_IntfId(intfId),
        m_setB(setChannels)
    {       
        // defaults
        m_IntIdPresent.Init( intfId.IsInited() );
        m_intType.Init(IeConstants::Other);
        m_prefExcl.Init(prefExcl);
        m_IsDchanInd.Init(false);
        m_chanSelect.Init(IeConstants::B1orIndInFollowing);
        m_codStd.Init(IeConstants::CCITT);
        m_numOrMap.Init(IeConstants::Num);
        m_chanType.Init(IeConstants::BChannels);
    }

    const SetBCannels& IeChannelIdentification::GetBchannels() const
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_setB;
    }

    IeConstants::ChanSelect IeChannelIdentification::GetChanSelection() const
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_chanSelect;
    }

    IeConstants::PrefExcl IeChannelIdentification::GetPrefExcl() const
    {
        ESS_ASSERT( m_prefExcl.IsInited() ); //TODO вызывающий знает что это поле должно быть?
        return m_prefExcl;
    }

    

    void IeChannelIdentification::WriteContent(std::stringstream& out) const
    {
        ESS_ASSERT(Mode() == readOnly);

        WriteField("CodingStandard", m_codStd, out);
        WriteField("Intf Id Present", m_IntIdPresent, out);
        WriteField("Intf Type", m_intType, out);
        WriteField("Pref/Excl", m_prefExcl, out);
        WriteField("Is D-chan Ind", m_IsDchanInd, out);
        WriteField("ChannelSelection", m_chanSelect, out);
        WriteField("NumOrMap", m_numOrMap, out);
        WriteField("ChannelType", m_chanType, out);                      
		out << "BChannels: " << m_setB.ToString();
    }        

    void IeChannelIdentification::Serialize(std::vector<byte>& raw) const // override
    {                        
        raw.push_back( GetId().Id() ); // Set identifier
        raw.push_back(0); // reserved len-field

        byte b = 0, ext;

        ext = 1 << 7;        
        if (m_IntIdPresent.get()) b = 1 << 6;
        if (m_IsDchanInd.get()) b |= 1 << 2;
        b |= ext | (m_intType << 5) | (m_prefExcl << 3) | m_chanSelect;
        raw.push_back(b);

        if (m_IntIdPresent.get())
        {            
            ESS_ASSERT(m_IntfId.IsInited());
            
            // supported single intfId only 
            ext = 1 << 7;
            b = ext | m_IntfId;            
            raw.push_back(b);
        }

        ext = 1 << 7;        
        b = ext | (m_codStd << 5) | (m_numOrMap << 4) | m_chanType;
        raw.push_back(b);
        
        for (int i = 0; i< m_setB.Capacity(); ++i)
        {   
            if (m_setB.IsExist(i))
                raw.push_back(i);            
        }
        ext = 1 << 7; // only for last element
        raw.back() = ext | raw.back();
        FixLen();                    
    }


    IeCalledPartyNumber::IeCalledPartyNumber(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
    {        
        AssertSizeAndId(raw, 4, infra);
        int len = raw[1];

        byte b = raw[2];

        int numType = ExtractValue(b, 4, 6);
        if (numType == 5) // numType never ==5
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        m_num.setType(IeConstants::TypeOfNum(numType));        

        IeConstants::NumPlan plan;
        CorrectSet converter;
        converter.Add(IeConstants::Data);
        converter.Add(IeConstants::Ext);
        converter.Add(IeConstants::ISDNtelephony);
        converter.Add(IeConstants::NationalPlan);
        converter.Add(IeConstants::Private);
        converter.Add(IeConstants::Telex);
        converter.Add(IeConstants::UnknownPlan);        
        if (!converter.TryConvert(ExtractValue(b, 0, 3), plan))
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));        
        m_num.setPlan(plan);

        std::string num;
        int numLen = len - 1;
        for(int i = 0; i < numLen; ++i)
        {
            num.push_back( raw[i + 3] );
        }

        if ( !Dss1Digits::IsCorrectCharset(num) )
        {
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        }

        // remove at end '#'
        if (CAddOctotorp && num.at(num.size() - 1) == '#') num.resize(num.size() - 1);

        m_num.setDigits( Dss1Digits(num) );

    }

    IeCalledPartyNumber::IeCalledPartyNumber(const DssPhoneNumber& num)
        : Base(writeOnly), m_num(num)
    {                        
    }        

    DssPhoneNumber IeCalledPartyNumber::GetNum()
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_num;
    }

    void IeCalledPartyNumber::WriteContent(std::stringstream& out) const
    {        
        ESS_ASSERT(Mode() == readOnly);
        WriteField("TypeOfNumber", Utils::EnumResolve(m_num.getType()), out);
        WriteField("Plan", Utils::EnumResolve(m_num.getPlan()), out);
        WriteField("Number", m_num.Digits().getAsString(), out);
    }

    void IeCalledPartyNumber::Serialize(std::vector<byte>& raw) const // override
    {        
        raw.push_back( GetId().Id() ); // Set identifier
        raw.push_back(0); // reserved len-field

        byte b, ext;

        // octet 3
        ext = 1 << 7;
        b = ext | (m_num.getType() << 4) | m_num.getPlan();
        raw.push_back(b);

        // octets 4        
        std::string nums = m_num.Digits().getAsString();
        if (CAddOctotorp && nums.at(nums.size() - 1) != '#') nums.append("#"); 
        std::copy(nums.begin(), nums.end(), std::inserter(raw, raw.end()) );

        FixLen();        
    }

    

    IeCallingPartyNumber::IeCallingPartyNumber(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
    {        
        AssertSizeAndId(raw, 3, infra); // min len
        int len = raw[1];        

        int currPos = 2;
        byte b = raw[currPos++];

        int numType = ExtractValue(b, 4, 6);
        if (numType == 5) // numType never ==5
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        m_num.setType( IeConstants::TypeOfNum(numType) );
        //m_typeOfNum.Init( IeConstants::TypeOfNum(numType) );

        IeConstants::NumPlan plan;
        CorrectSet converter;
        converter.Add(IeConstants::Data);
        converter.Add(IeConstants::Ext);
        converter.Add(IeConstants::ISDNtelephony);
        converter.Add(IeConstants::NationalPlan);
        converter.Add(IeConstants::Private);
        converter.Add(IeConstants::Telex);
        converter.Add(IeConstants::UnknownPlan);
        if (!converter.TryConvert(ExtractValue(b, 0, 3), plan))
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        //m_plan.Init(plan);
        m_num.setPlan(plan);

        byte ext = ExtractValue(b, 7, 7);            
        if (ext == 0) // octet 3a present
        {
            b = raw[currPos++]; // =raw[3]
            m_prInd.Init( IeConstants::PresentInd(ExtractValue(b, 5, 6)) );
            m_screenInd.Init( IeConstants::ScreeningInd(ExtractValue(b, 0, 1)) );                
        }

        std::string num;
        for(; currPos < len + 2; ++currPos)// в последней цифре ст бит(ext) == 1?
        {
            num.push_back(raw[currPos]);
        }

        if (!Dss1Digits::IsCorrectCharset(num))
        {
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        }

        m_num.setDigits( Dss1Digits(num) );        
    }

    IeCallingPartyNumber::IeCallingPartyNumber(const DssPhoneNumber& num)
        : Base(writeOnly), m_num(num)
    {                
        // default (TODO remove to constructor)
        m_screenInd.Init(IeConstants::UserProv_NotScreened); 
        m_prInd.Init(IeConstants::Allowed);
    }    
   
    IeConstants::PresentInd IeCallingPartyNumber::GetPresentInd()
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_prInd;
    }

    DssPhoneNumber IeCallingPartyNumber::GetNum()
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_num;
    }

    IeConstants::ScreeningInd IeCallingPartyNumber::GetScreeningInd()
    {
        ESS_ASSERT(Mode() == readOnly);
        return m_screenInd;
    }

    void IeCallingPartyNumber::WriteContent(std::stringstream& out) const
    {
        ESS_ASSERT(Mode() == readOnly);

        WriteField("TypeOfNumber", Utils::EnumResolve(m_num.getType()), out);
        WriteField("Plan", Utils::EnumResolve(m_num.getPlan()), out);
        WriteField("PresentInd", m_prInd, out);
        WriteField("ScreeningInd", m_screenInd, out);
        WriteField("Number", m_num.Digits().getAsString(), out);        
    }

    void IeCallingPartyNumber::Serialize(std::vector<byte>& raw) const // override
    {        
        raw.push_back( GetId().Id() ); // Set identifier
        raw.push_back(0); // reserved len-field

        byte b, ext;

        // octet 3
        m_prInd.IsInited() ? ext = 0 : ext = 1 << 7;
        b = ext | (m_num.getType() << 4) | m_num.getPlan();
        raw.push_back(b);

        // octet 3a
        if (m_prInd.IsInited())
        {
            ext = 1 << 7;
            b = ext | (m_prInd << 5) | m_screenInd;
            raw.push_back(b);
        }

        // octets 4        
        const std::string& nums = m_num.Digits().getAsString();
        std::copy(nums.begin(), nums.end(), std::inserter(raw, raw.end()) );

        FixLen();        
    }    

    IeCause::IeCause(const RawData& raw, IsdnInfra& infra) : Base(readOnly), m_pInfra(&infra)
    {                    
        AssertSizeAndId(raw, 4, infra);
        int len = raw[1];
        
        if (len > 30 || len == 0)
        {
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        }        

        int skip = 0;
        byte b = raw[2];
        m_location.Init( IeConstants::Location(ExtractValue( b,0,3 )) );
        m_codStd.Init( IeConstants::CodingStandard(ExtractValue( b,5,6 )) );
        if (!ExtractValue(b,7,7)) ++skip;// skip recomendation
        b = raw[3 + skip];

        //TODO!Возможно неверное преобразоание, если полученной causeN нет в enum!
        m_causeNum.Init( IeConstants::CauseNum( ExtractValue(b,0,6) ) ); 

        // if diagnostic exist
        if (raw.Size() > 4 + skip) // (len > 2 + skip) 
        {
            b = raw[ 4 + skip ];
            m_dignosticValue.Init(b);
        }                                
    }

    IeCause::IeCause( shared_ptr<const DssCause> pCause )  : Base(writeOnly), m_pInfra(0)
    {            
        m_causeNum.Init( pCause->GetCauseNum() );
        m_codStd.Init( pCause->GetStandart() ); //default или вынести в профайл
        m_location.Init( pCause->GetLocation() );
        //m_dignosticValue.Init(pCause-> -=Unsupported-= )
    }

    /*int GetCauseNum() const { return m_causeNum; }
    CodingStandard GetCodStd() const { return m_codStd; }
    Location GetLocation() const { return m_location; }*/

    shared_ptr<const DssCause> IeCause::GetCause()
    {
        ESS_ASSERT(m_pInfra);            
        return DssCause::Create(*m_pInfra, m_location, m_codStd, m_causeNum);
    }

    void IeCause::WriteContent(std::stringstream& out) const
    {
        ESS_ASSERT(Mode() == readOnly);
        WriteField("Cause No", m_causeNum, out);
        WriteField("DiagVal", m_dignosticValue, out);
        WriteField("CodingStandard", m_codStd, out);
        WriteField("Location", m_location, out);            
    }       

    void IeCause::Serialize(std::vector<byte>& raw) const // override
    {        
        raw.push_back( GetId().Id() );
        raw.push_back(0); // reserved len-field

        int ext = 1 << 7;
        byte b = ext | (m_codStd << 5) | m_location;
        raw.push_back(b);

        b = ext | m_causeNum; 
        raw.push_back(b);

        // if (m_dignosticValue.IsInited()) raw.push_back(m_dignosticValue);
        // not supported in DssCause

        FixLen();        
    }

    IeProgressInd::IeProgressInd(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
    {                   
        AssertSizeAndId(raw, 4, infra);
        int len = raw[1];            
        if (len > 2)
        {
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        }            

        CorrectSet converter;

        byte b = raw[2];
        IeConstants::Location loc;
        converter.Add(IeConstants::BI);
        converter.Add(IeConstants::INTL);
        converter.Add(IeConstants::LN);
        converter.Add(IeConstants::LPN);
        converter.Add(IeConstants::RLN);
        converter.Add(IeConstants::RPN);
        converter.Add(IeConstants::TN);
        converter.Add(IeConstants::U);
        if ( !converter.TryConvert(ExtractValue( b,0,3 ), loc) )
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        m_location.Init(loc);

        m_codStd.Init( IeConstants::CodingStandard(ExtractValue( b,5,6 )) );

        b = raw[3];
        IeConstants::ProgressDscr descr;
        converter.Reset();
        converter.Add(IeConstants::DestAdrNonIsdn);
        converter.Add(IeConstants::HasRetToIsdn);
        converter.Add(IeConstants::InBand);
        converter.Add(IeConstants::Interworking);
        converter.Add(IeConstants::NotEndToEndISDN);
        converter.Add(IeConstants::OrigAdrNonIsdn);
        if ( !converter.TryConvert(ExtractValue( b,0,6 ), m_descr) )
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        
    }

    IeProgressInd::IeProgressInd( IeConstants::Location loc, IeConstants::ProgressDscr descr,
        IeConstants::CodingStandard codStd) : Base(writeOnly)
    {            
        m_codStd.Init(codStd);
        m_location.Init(loc);
        m_descr = descr;
    }

    void IeProgressInd::WriteContent(std::stringstream& out) const
    {
        ESS_ASSERT(Mode() == readOnly);                       
        WriteField("CodingStandard", m_codStd, out);
        WriteField("Location", m_location, out);        
        WriteField("Description", Utils::EnumResolve(m_descr), out);              
    }

    void IeProgressInd::Serialize(std::vector<byte>& raw) const // override
    {
        ESS_ASSERT( raw.size()==0 );            
        raw.push_back( GetId().Id() );
        raw.push_back(0); // reserved len-field

        int ext = 1 << 7;
        byte b = ext | (m_codStd << 5) | m_location;
        raw.push_back(b);

        ext = 1 << 7;
        b = ext | m_descr; 
        raw.push_back(b);

        FixLen();        
    }

    //-------------------------------------------------------------------------------------

    IeConstants::ProgressDscr IeProgressInd::getDescr() const
    {
        return m_descr;
    }

    //-------------------------------------------------------------------------------------

    IeSignal::IeSignal(const RawData& raw, IsdnInfra& infra) : Base(readOnly)
    {        
        AssertSizeAndId(raw, 3, infra); // min len

        int len = raw[1];        

        if (len != 1)
        {
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        }            

        CorrectSet converter;

        byte b = raw[2];
        IeConstants::SignalValue val;

        converter.Add(IeConstants::DialTone);
        converter.Add(IeConstants::RingBackToneOn);
        converter.Add(IeConstants::InterceptToneOn);
        converter.Add(IeConstants::NetCongestionToneOn);
        converter.Add(IeConstants::BusyToneOn);
        converter.Add(IeConstants::ConfirmToneOn);
        converter.Add(IeConstants::AnswToneOn);
        converter.Add(IeConstants::CallWaitingToneOn);
        converter.Add(IeConstants::OffHookWarning);
        converter.Add(IeConstants::PreemptionToneOn);
        converter.Add(IeConstants::TonesOff);
        converter.Add(IeConstants::AllertingOn_p0);
        converter.Add(IeConstants::AllertingOn_p1);
        converter.Add(IeConstants::AllertingOn_p2);
        converter.Add(IeConstants::AllertingOn_p3);
        converter.Add(IeConstants::AllertingOn_p4);
        converter.Add(IeConstants::AllertingOn_p5);
        converter.Add(IeConstants::AllertingOn_p6);
        converter.Add(IeConstants::AllertingOn_p7);
        converter.Add(IeConstants::AllertingOff);

        if ( !converter.TryConvert(b, val) )
        {
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        }

    }

    IeSignal::IeSignal(IeConstants::SignalValue val) : Base(writeOnly)
    {
        m_signalValue.Init(val);
    }

    void IeSignal::WriteContent(std::stringstream& out) const
    {        
        ESS_ASSERT(Mode() == readOnly);
        WriteField("SignalValue", m_signalValue, out);        
    }

    void IeSignal::Serialize(std::vector<byte>& raw) const
    {
        ESS_ASSERT( raw.size() == 0 );        

        raw.push_back( GetId().Id() ); // Set identifier [0]
        raw.push_back(1);                                // [1]
        raw.push_back(m_signalValue);                    // [2]               
    }

    // ------------------------------------------------------------------------------------

    IeRestartInd::IeRestartInd( const RawData& raw, IsdnInfra& infra )  : Base(readOnly)
    {        
        AssertSizeAndId(raw, 3, infra); // min len

        int len = raw[1];        

        if (len != 1)
        {
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        }            

        CorrectSet converter;

        byte b = raw[2];        

        converter.Add(IeConstants::Channels);
        converter.Add(IeConstants::SingleIntf);
        converter.Add(IeConstants::AllIntf);

        if ( !converter.TryConvert(ExtractValue( b,0,2 ), m_val) )
        {
            ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( IeException::Create(infra) ));
        }
    }

    IeRestartInd::IeRestartInd( IeConstants::RestartClass val ) : Base(writeOnly) 
    {
        m_val = val;
    }

    void IeRestartInd::WriteContent( std::stringstream& out ) const
    {
        ESS_ASSERT(Mode() == readOnly);
        WriteField("RestartClass", Utils::EnumResolve(m_val), out);        
    }

    void IeRestartInd::Serialize(std::vector<byte>& raw) const /*  */
    {
        ESS_ASSERT( raw.size() == 0 );
       

        raw.push_back( GetId().Id() );    // Set identifier [0]
        raw.push_back(1);                                // [1]
        
        byte b, ext;
        ext = 1 << 7;
        b = ext | m_val;  
        raw.push_back(b);                                // [2]        
    }

    IeConstants::RestartClass IeRestartInd::getValue() const
    {
        return m_val;
    }

    // ------------------------------------------------------------------------------------
    

    UnknownIeType::UnknownIeType(IeId id, const RawData& raw, IsdnInfra& infra ) : Base(readOnly)
    {        
        int size = (raw.Size() >= 2 && raw.Size() >= raw[1]) ? raw[1] : raw.Size();        
        for (int i = 0; i < size; ++i)
        {
            m_dump.push_back(raw[i]);
        }
        m_id = id;
    }

    void UnknownIeType::WriteContent( std::stringstream& out ) const
    {
        WriteField("CodeSet", Utils::IntToString(m_id.Codset()), out);
        WriteField("Code", Utils::IntToString(m_id.Id()), out);       
      
        out << "\nDump: ";
        out.setf(std::ios::uppercase);
        for(int i = 0; i < m_dump.size(); ++i)
        {
            out.width(2);
            out.fill('0');
            out << std::hex << (int)m_dump[i++] << " ";
            //if(i%20 == 0) out << "";
        }
    }

    void UnknownIeType::Serialize( std::vector<byte>& raw ) const
    {
        ESS_HALT("Create pack from this Ie not accepted");
    }
} // namespace ISDN



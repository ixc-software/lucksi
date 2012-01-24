#pragma once

#include "stdafx.h"
#include "Utils/ErrorsSubsystem.h"
#include "Utils/BaseSpecAllocated.h"
#include "IIsdnAlloc.h"
#include "SetBCannels.h"

#include "IeConstants.h"
#include "BChannelsWrapper.h"
//#include "IsdnUtils.h"
#include "Utils/InitVar.h"


namespace ISDN
{        
    
    class Dss1Digits
    {
        std::string m_digits;        

    public:        

        explicit Dss1Digits(const std::string& digits) : m_digits(digits)
        {            
            ESS_ASSERT( IsCorrectCharset(m_digits) );
        }        

        static bool IsCorrectCharset(const std::string& val)
        {            
            const char charset[] = "*#1234567890";

            return (std::string::npos == val.find_first_not_of(charset, 0));
        }

        bool operator==(const Dss1Digits& rhs) const 
        {
            return m_digits == rhs.m_digits;
        }        

        bool operator !=(const Dss1Digits& rhs) const 
        {
            return !operator==(rhs);
        }
        

        Dss1Digits& Append(const Dss1Digits& other)
        {
            m_digits.append(other.m_digits);
            return *this;
        }  

        bool IsEmpty() const 
        {
            return m_digits.empty();
        }

        const std::string &getAsString() const
        {
            return m_digits;
        }

    };

    // номер вызываемый, вызывающий   
    class DssPhoneNumber
    {
    public:

        DssPhoneNumber(const std::string& num = "", 
                IeConstants::TypeOfNum type = IeConstants::UnknownType, 
                IeConstants::NumPlan plan = IeConstants::UnknownPlan) 
            : m_digits(num),
            m_type(type),
            m_plan(plan)
        {            
        }

        void setDigits(const Dss1Digits& num)
        {
            ESS_ASSERT( m_digits.IsEmpty() );
            m_digits = num;            
        }
        
        const Dss1Digits& Digits() const 
        {
            return m_digits;
        }

        void setType(IeConstants::TypeOfNum type)
        {
            m_type = type;
        }

        void setPlan(IeConstants::NumPlan plan)
        {
            m_plan = plan;
        }

        IeConstants::TypeOfNum getType() const 
        {
            return m_type;
        }

        IeConstants::NumPlan getPlan() const
        {
            return m_plan;
        }        

        void AppendDigits(const Dss1Digits& digits)
        {
            m_digits.Append(digits);
        }

    private:               

        Dss1Digits m_digits;
        IeConstants::TypeOfNum m_type;
        IeConstants::NumPlan m_plan;
        
    };

    // ------------------------------------------------------------
   
    class ServiceInd 
    {
        /*???*/        
    };

    // ------------------------------------------------------------

    /*
    TODO: 
    - все параметры вызова скомпоновать в структуры по назначению (прим SetupParams)
    - опциональные или все (для однообразия) параметры этих структур хранить через 
    InitPrp<> (альтернативно конструктору. легко обеспечить смесь обязательных и опциональных параметров, уйдет дублирование в Create итп)
    - In/OutCallParams сделать структурами содержащими открытые поля типа SetupParams
    */

    // параметры вызова    
    class DssCallParams : public Utils::BaseSpecAllocated<IIsdnAlloc>
    { 
    public:

        virtual ~DssCallParams(){}
        
        // вызываемый
        const DssPhoneNumber& GetCalled() const { return m_calledNum; }
       
        const IeConstants::TransMode& GetMode() const { return m_mode; }

        const IeConstants::TransCap& GetTransCap() const { return m_transCap; }

        const IeConstants::TransRate& GetRate() const { return m_rate; }
       
        const ServiceInd& GetServInd() const {return m_servInd;}
              
        int GetReqBchansCount() const {return m_ReqBchsCount;}
        
        // вызывающий
        const DssPhoneNumber& GetCalling() const { return m_callingNum;}          
        
		const Utils::InitProp<int> &InterfaceNumber() const {	return m_intfId; }
        const SetBCannels& GetAckBchans() const {return m_BchsAck;}

        IeConstants::ProgressDscr GetProgressDescription() const { return m_progressDescr; }


        std::string ToString() const
        {
            std::stringstream ss;
            ss << "CalledNum: " << m_calledNum.Digits().getAsString() << ";"
               << " CallingNum: " << m_callingNum.Digits().getAsString() << ";"
               << " Requested B-channels count: " << m_ReqBchsCount << ";"
               << " Assigned channels: " << m_BchsAck.ToString() << ".";

            return ss.str();
        }

    protected:

        DssCallParams(const DssPhoneNumber& calledNum = DssPhoneNumber(), 
            DssPhoneNumber callingNum = DssPhoneNumber(),
            ServiceInd servInd = ServiceInd(),
            IeConstants::TransMode mode = IeConstants::Circuit,
            IeConstants::TransCap transCap = IeConstants::AUDIO_3_1K,
            IeConstants::TransRate rate = IeConstants::KbPs64,
            int reqChannelsCount = 1,
            IeConstants::ProgressDscr progressDescr = IeConstants::DestAdrNonIsdn)
            : m_calledNum(calledNum), 
            m_callingNum(callingNum),
            m_servInd(servInd),
            m_mode(mode), 
            m_transCap(transCap),
            m_rate(rate),
            m_ReqBchsCount(reqChannelsCount),
            m_progressDescr(progressDescr)            
        {}

        void SetProgressDscr(IeConstants::ProgressDscr descr)
        {
            m_progressDescr = descr;
        }

        void SetCalled(const DssPhoneNumber& num)
        {            
            m_calledNum = num;
        }

        void SetMode(const IeConstants::TransMode& mode)
        {            
            m_mode = mode;
        }

        void SetTransCap(const IeConstants::TransCap& cap)
        {            
            m_transCap = cap;
        }

        void SetRate(const IeConstants::TransRate& rate)
        {            
            m_rate = rate;
        }

        void SetServInd(const ServiceInd& ind)
        {            
            m_servInd = ind;
        }

        void SetReqBchansCount(int count)
        {            
            m_ReqBchsCount = count;
        }

        void SetCalling(const DssPhoneNumber& callingNum)
        {
            m_callingNum = callingNum;
        }

        void SetAckBchans(const BChannelsWrapper& ackBchs)
        {
            m_BchsAck = ackBchs.getChans();
            m_intfId.Init( ackBchs.getIntfId() );
        }

    private:

        DssPhoneNumber m_calledNum;
        DssPhoneNumber m_callingNum;
        ServiceInd m_servInd; // ?
        IeConstants::TransMode m_mode;
        IeConstants::TransCap m_transCap;
        IeConstants::TransRate m_rate;
        int m_ReqBchsCount; // количество запрашиваемых каналов
        SetBCannels m_BchsAck; // назначенные каналы (в ответе)
        Utils::InitProp<int> m_intfId;
        IeConstants::ProgressDscr m_progressDescr;
        
    };

   
    class DssOutCallParams : public DssCallParams
    {
        DssOutCallParams(const DssPhoneNumber& calledNum, 
            DssPhoneNumber callingNum,
            ServiceInd servInd,
            IeConstants::TransMode mode, IeConstants::TransCap cap,
            IeConstants::TransRate rate, int reqChannelsCount
            ) :
            DssCallParams(calledNum, callingNum, servInd, mode, cap, rate, reqChannelsCount)
        {}

        DssOutCallParams( const DssCallParams& params) : DssCallParams(params) {}

    public:

        static DssOutCallParams* Create(IIsdnAlloc& alloc,
            const DssPhoneNumber& calledNum, 
            const DssPhoneNumber& callingNum = DssPhoneNumber(),
            ServiceInd servInd = ServiceInd(),
            IeConstants::TransMode mode = IeConstants::Circuit,
            IeConstants::TransCap transCap = IeConstants::AUDIO_3_1K,
            IeConstants::TransRate rate = IeConstants::KbPs64,
            int reqChannelsCount = 1,
            IeConstants::ProgressDscr progressDescr = IeConstants::DestAdrNonIsdn)
        {
            return new(alloc) 
                DssOutCallParams(calledNum, callingNum, servInd, mode, transCap, rate, reqChannelsCount);
        }

        static DssOutCallParams* Create(IIsdnAlloc& alloc,const DssCallParams& params)
        {
            return new(alloc) DssOutCallParams(params);
        }        

    };

} // ISDN






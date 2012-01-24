#ifndef DSSCAUSE_H
#define DSSCAUSE_H

#include "stdafx.h"
#include "IeConstants.h"
#include "IsdnUtils.h"
#include "IIsdnL3Internal.h"
#include "Utils/BaseSpecAllocated.h"
#include "ListIe.h" // не нужен, если не реализовывать диагностику

namespace ISDN
{
    using IsdnUtils::CorrectSet;
    using boost::shared_ptr;

    // объект описания причины действия или отказа
    // соответствует q931 cause по содержанию.
    class DssCause : public Utils::BaseSpecAllocated<IIsdnAlloc>, boost::noncopyable
    {
        IeConstants::Location m_location;
        IeConstants::CodingStandard m_codStd;
        IeConstants::CauseNum m_causeNum;
        //TODO Add ListIe, recomendation, condition
        
        DssCause(IeConstants::Location location, IeConstants::CodingStandard codingStandart, IeConstants::CauseNum causeNum):
          m_location(location), m_codStd(codingStandart), m_causeNum(causeNum)
        {
            /*CorrectSet validate;
            validate.Add(IeConstants::BI);
            validate.Add(IeConstants::INTL);
            validate.Add(IeConstants::LN);
            validate.Add(IeConstants::LPN);
            validate.Add(IeConstants::RLN);
            validate.Add(IeConstants::RPN);
            validate.Add(IeConstants::TN);
            validate.Add(IeConstants::U);

            ESS_ASSERT( validate.TryConvert(location, m_location) );*/            
        }

    public:

        std::string ToString() const
        {
            // TODO decode int`s to string
            std::stringstream ss;
            ss << "Cause No:" << m_causeNum << ", source location code: " << m_location;
            return ss.str();
        }

        IeConstants::Location GetLocation() const
        {
            return m_location;
        }

        IeConstants::CodingStandard GetStandart()const
        {
            return m_codStd;
        }

        IeConstants::CauseNum GetCauseNum()const
        {
            return m_causeNum;
        }

        int GetClass() const 
        {
            return ((m_causeNum & 0x70) >> 4);
        }

        static shared_ptr<const DssCause> Create(IIsdnAlloc& alloc, 
            IeConstants::Location location, IeConstants::CodingStandard codingStandart, IeConstants::CauseNum causeNum)
        {
            return shared_ptr<const DssCause>( new(alloc) DssCause(location, codingStandart, causeNum) );
        }

        //TODO replce func body to cpp, because use redir. by FFdeclare class IIsdnL3Internal
        static shared_ptr<const DssCause> Create(IIsdnL3Internal* pIL3, IeConstants::CauseNum causeNum)
        {
            return shared_ptr<const DssCause>( new(pIL3->GetInfra()) DssCause(pIL3->GetOptions().m_Location, IeConstants::CCITT, causeNum) );
        }

        bool operator == (const DssCause& rhs)const
        {
            return ( m_causeNum == rhs.m_causeNum &&
                m_codStd == rhs.m_codStd &&
                m_location == rhs.m_location);
        }

    };

    // Класс-исключение для передачи Cause через throw
    //ESS_TYPEDEF_T(DssCauseAsExcept, boost::shared_ptr<const DssCause> );

} // ISDN

#endif


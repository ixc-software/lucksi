#include "stdafx.h"
#include "Utils/FullEnumResolver.h"
#include "L3Packet.h"
#include "CallRef.h"
#include "IsdnUtils.h"

namespace ISDN
{     
    using boost::shared_ptr;
    using IsdnUtils::ShortName;

	L3Packet::L3Packet (const CallRef &callref, IIsdnL3Internal& IL3, L3MessgeType::Type type) : 
        m_infra( IL3.GetInfra() ),
        m_ListIe( IL3.GetInfra(), IL3.getIeFactory() ),
        m_incomming(false),
		m_callref(callref)
    {
        // add Protocol discriminator to packet header
        m_IeData.push_back(CProtDescriptor);

        // add callref to packet header
        m_callref.AddBackCRef(m_IeData);
		m_type = type;
        // add message type
        m_IeData.push_back(type);      
        
    }

	//----------------------------------------------------------------------------------------

	L3Packet::L3Packet (QVector<byte> l2data, IsdnInfra& infra, IeFactory& ieFactory) : 
		m_infra( infra ), m_ListIe( infra, ieFactory ),
        m_incomming(true),
		m_callref(l2data, m_infra)
	{

		//fatal assert, тк эту ошибку должна была отловить DataInd
		ESS_ASSERT( l2data.size() >= 3 && "Wrong IPacket in L3Packet construct");

		// пока ничего не выделяли и не создавали можно throw
		if (IsBroadcast(l2data))
		{            
			ESS_THROW_T(DssWarningExcept, shared_ptr<DssWarning>( UnsupportedBroadcast::Create(m_infra) ));		
        }        
		// TODO oter condition ...     

		m_type = ExtractMessageType(m_infra, l2data);

		// парсим, заполняя list(проверка Mt-Ie в конкретном Ie)                                      
		m_ListIe.Parse(l2data);              
	}

    //----------------------------------------------------------------------------------------

    const std::string& L3Packet::GetAsString(bool withIeContent) const
    {
        if (!m_myContent.empty()) return m_myContent;            

        if (!m_incomming)
        {                
            QVector<byte> qData;
            Utils::Converter<>::DoConvert(m_IeData, qData);
            m_ListIe.Parse(qData);
        }

		m_myContent = "Type: ";
		m_myContent += GetName() + ". Call ref: " + m_callref.ToString() + 
			".\n " + m_ListIe.GetAsString(withIeContent);
        return m_myContent;
    }        

    //----------------------------------------------------------------------------------------

    const std::string& L3Packet::GetName()const
    {
        if(m_myName.empty())
		{
            m_myName = Utils::EnumResolve(m_type);			
        }
        return m_myName;
    }

    //----------------------------------------------------------------------------------------   

    void L3Packet::AddIe(const ICommonIe& Ie)
    {
        ESS_ASSERT ( !m_incomming );
        IeId id = Ie.GetIeId();
        RememberAdded(id);
        
        ESS_ASSERT(IeCorrectForThisMsgType(id) && "Incompatible Ie Mt");        
        
        const VectorByte &tmp( Ie.Get() );
        std::copy(tmp.begin(), tmp.end(), std::inserter(m_IeData, m_IeData.end()) );
    }

    //----------------------------------------------------------------------------------------   

    bool L3Packet::ValidateMandatoryIes()
    {
        ESS_ASSERT(m_incomming);
        std::multiset<IeId> existIe;
        m_ListIe.GetExistId(existIe);
        return ValidateMandatoryIes( existIe );
    }

    //----------------------------------------------------------------------------------------   

    L3Packet::VectorByte L3Packet::GetRaw() const
    {
        ESS_ASSERT(!m_incomming);
        ValidateMandatoryIes(m_addedIes);
        return m_IeData;
    }

	//----------------------------------------------------------------------------------------   

	L3MessgeType::Type L3Packet::ExtractMessageType(IIsdnAlloc& alloc, const QVector<byte> &data)  
	{  
		
		int crv_len = data.at(1) & 0x0f;
        if (data.size() <= (2 + crv_len) )
        {
            boost::shared_ptr<DssWarning> err(PackWithUnknounMt::Create(alloc, data) );
            ESS_THROW_T(DssWarningExcept, err);
        }
		return L3MessgeType::Type(data.at(2 + crv_len));
	}

} // ISDN


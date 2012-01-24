#include "stdafx.h"
#include "L3Profiles.h"
#include "NObjL3Profile.h"
#include "Domain/DomainClass.h"

#include "Utils/QtEnumResolver.h"

namespace ISDN
{
    NObjL3Profile::NObjL3Profile( Domain::NamedObject *pParent, const Domain::ObjectName &name,
		IeConstants::SideType side) : 
		Domain::NamedObject(&pParent->getDomain(), name, pParent)
    {
        m_pTraceOptions = new NObjDssTraceOption(this, "Trace");
        m_sideType = side;
        m_statActive = true;
		SetDefault(m_sideType);
    }    

    // ------------------------------------------------------------------------------------

    Q_INVOKABLE void NObjL3Profile::SetDefault(IeConstants::SideType side)
    {
        m_sideType = side;

        L3Profile::Options& option = *this;        
        option.SetSide(side == IeConstants::NT);
        
        if (side == IeConstants::NT) 
            m_timers.InitNetDefault();
        else
            m_timers.InitUserDefault();
    }

    // ------------------------------------------------------------------------------------

	boost::shared_ptr<L3Profile> NObjL3Profile::GenerateProfile( Utils::SafeRef<IDssToGroup> intfGroup, 
		const std::string &nameDss1)
    {
        boost::shared_ptr<L3Profile> result = (m_sideType == IeConstants::NT) ? 
			L3Profile::CreateAsNet(*m_pTraceOptions, intfGroup, nameDss1, m_statActive) : 
			L3Profile::CreateAsUser(*m_pTraceOptions, intfGroup, nameDss1, m_statActive);

        result->GetOptions() = *this;
        result->GetTimers() = m_timers;

        return result;
    }

    // ------------------------------------------------------------------------------------

    QString NObjL3Profile::getLocation() const
    {
        Utils::QtEnumResolver<IeConstants, IeConstants::Location> resolver;
        return resolver.Resolve(m_Location).c_str();
    }

    // ------------------------------------------------------------------------------------

    void NObjL3Profile::setLocation( QString enumVal )
    {
        Utils::QtEnumResolver<IeConstants, IeConstants::Location> resolver;
        m_Location = resolver.Resolve(enumVal.toStdString().c_str());
    }

    // ------------------------------------------------------------------------------------    

	void NObjL3Profile::Side(const QString &side)
	{
		Utils::QtEnumResolver<IeConstants, IeConstants::SideType> resolver;
		m_sideType = resolver.Resolve(side.toStdString().c_str());
	}

	// ------------------------------------------------------------------------------------

	QString NObjL3Profile::Side() const
	{
		Utils::QtEnumResolver<IeConstants, IeConstants::SideType> resolver;
		return resolver.Resolve(m_sideType).c_str();
	}
    
} // namespace ISDN

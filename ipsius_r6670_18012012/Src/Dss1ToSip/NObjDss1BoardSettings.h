#pragma once

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"

#include "iCmp/ChMngProto.h"
#include "iCmpExt/NObjCmpDevSettings.h"

namespace Dss1ToSip
{
	
	class NObjDss1BoardSettings : public iCmpExt::NObjCmpDevSettings
	{
		Q_OBJECT;

	public:

		NObjDss1BoardSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name,
			boost::shared_ptr<iCmp::BfInitDataBase> initData) :
			iCmpExt::NObjCmpDevSettings(pParent, name, *initData),
			m_initData(initData)
		{
			PropertyWriteEvent(pParent);
			*m_initData = iCmp::BfInitDataBase::DefaultValue();
		}
		
		const iCmp::BfInitDataBase& Settings() const
		{
			return *m_initData;
		}

	private:
		boost::shared_ptr<iCmp::BfInitDataBase> m_initData;

	};
}




#pragma once

#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "DRI/INonCreatable.h"
#include "Ds2155/liuProfile.h"
#include "Dss1Def.h"

namespace Dss1ToSip
{
	class NObjLiuProfile : 
        public Domain::NamedObject, 
		public DRI::INonCreatable,
		Ds2155::LiuProfile
	{
		Q_OBJECT;

	public:

		NObjLiuProfile(Domain::NamedObject *pParent, 
			HardType::Value type, 
			const Domain::ObjectName &name = "Liu") :
			NamedObject(&pParent->getDomain(), name, pParent)
		{
			PropertyWriteEvent(pParent);
			(type == HardType::T1) ? SetAsT1() : SetAsPcm31();
		}

        Q_PROPERTY(Ds2155::Enums::LineCodeType LineCode READ m_lineCode WRITE m_lineCode);
        Q_PROPERTY(Ds2155::Enums::CycleType Cycle READ m_cycle WRITE m_cycle);       
        Q_PROPERTY(bool CRC4 READ m_enableCrc4 WRITE m_enableCrc4);      
        Q_PROPERTY(int MFAS READ m_MFAS WRITE m_MFAS);
        Q_PROPERTY(int FAS  READ m_FAS  WRITE m_FAS);
        Q_PROPERTY(int NFAS READ m_NFAS WRITE m_NFAS);
        Q_PROPERTY(bool SyncMaster READ m_syncMaster WRITE m_syncMaster);
		LiuProfile Profile() const
		{
			return *this;  // умышленная срезка типа 
		}

	};

    // -----------------------------------------------

    // for testing
    class NObjLiuProfileHolder : public Domain::NamedObject
    {
        Q_OBJECT;

        NObjLiuProfile &m_profile;
        bool m_enabled;

    public:
        NObjLiuProfileHolder(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
			Domain::NamedObject(pDomain, name),
			m_profile( *(new NObjLiuProfile(this, HardType::E1)) )
        {            
            m_enabled = true;
        }

        Q_PROPERTY(bool Enabled READ m_enabled WRITE SetEnabled);      

        void SetEnabled(bool val)
        {
            if (val == m_enabled) return;

            m_enabled = val;
            if (m_enabled) m_profile.PropertyWriteEventClear();
                      else m_profile.PropertyWriteEventAbort("NObjLiuProfileHolder is inactive");
        }


    };

}




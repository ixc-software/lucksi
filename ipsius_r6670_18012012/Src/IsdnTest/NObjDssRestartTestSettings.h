#ifndef NOBJDSSRESTARTTESTSETTINGS_H
#define NOBJDSSRESTARTTESTSETTINGS_H

#include "NObjDssTestSettings.h"
#include "SimulLiveStatistic.h"

namespace IsdnTest
{
    class NObjDssRestartTestSettings
        : public Domain::NamedObject,
        public DRI::INonCreatable
    {
        Q_OBJECT  
    public:
        NObjDssTestSettings *const m_pGeneralSettings;
        bool m_restartFromUserSide; 

        
        NObjDssRestartTestSettings(Domain::NamedObject *pParent, const Domain::ObjectName &name)
            : Domain::NamedObject(&pParent->getDomain(), name, pParent),
            m_pGeneralSettings(new NObjDssTestSettings(this, "GeneralProfile")),
			m_restartFromUserSide(false)
        {
            m_pGeneralSettings->m_MaxTestTimeout = 5000;
        }

        Q_PROPERTY(bool RestartFromUserSide READ m_restartFromUserSide WRITE m_restartFromUserSide);

    };
}
#endif

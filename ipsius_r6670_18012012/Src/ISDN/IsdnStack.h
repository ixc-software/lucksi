#ifndef ISDN_ST_H
#define ISDN_ST_H

#include "stdafx.h"
#include "Domain/IDomain.h"
#include "Utils/ErrorsSubsystem.h"
#include "IsdnLayersInterfaces.h"
#include "isdninfr.h"
#include "DssUpIntf.h"
#include "IsdnStackSettings.h"
#include "L2StatData.h"
#include "L3StatData.h"
#include "Dss1Binders.h"
#include "isdnl2.h"
#include "isdnl3.h"

namespace ISDN
{
   
    using Domain::IDomain;
    using boost::scoped_ptr;

    class IsdnL2;
    class IsdnL3;

    // сборка драйвера, LAPD, Dss в единое целое.
    // Возможность перезагрузки "средних уровней" (2,3)
    class IsdnStack : boost::noncopyable
	{
        enum
        {
            CDefaultProcessPeriod = 50, // msec
        };

	public:
		
        IsdnStack(IDomain& iDomain, const IsdnStackSettings& prof, IsdnInfra& infr, BinderToIDriver drv, BinderToICallBackDss pUpIntf);

        ~IsdnStack();		

		IsdnL2 * GetL2Ptr();

		IsdnL3 * GetL3Ptr();

        ILayerDss* GetDssIntf();

        IeConstants::Location GetLocation();        

        void Start(int period = CDefaultProcessPeriod);        

        void  Stop();

        void SetNewDriver(BinderToIDriver drvBinder);

        void ReloadL2(const shared_ptr<const L2Profile> newProfile);

        void ReloadL3(const shared_ptr<const L3Profile> newProfile);

        void SetNewUser(BinderToICallBackDss userBinder);

        L2StatData getL2Stat() const;

        void ClearL2Stat();

        L3StatData getL3Stat() const;

        void ClearL3Stat();

	private: 

        IDomain& m_iDomain;        
        BinderToIDriver m_drvBinder;

        BinderToICallBackDss m_usrBinder;        
		
		scoped_ptr<IsdnL2> m_pL2;
		scoped_ptr<IsdnL3> m_pL3;		
		IsdnInfra& m_infr; // utilz instance
	};

} // namespace ISDN

#endif






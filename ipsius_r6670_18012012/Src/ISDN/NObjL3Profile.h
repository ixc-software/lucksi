#ifndef NOBJL3PROFILE_H
#define NOBJL3PROFILE_H

#include "L3Profiles.h"
#include "NObjDssTraceOption.h"
#include "Domain/NamedObject.h"

namespace ISDN
{
    
    class NObjL3Profile : public Domain::NamedObject,
		public DRI::INonCreatable,
        public L3Profile::Options
    {   
        Q_OBJECT;

    // DRI interface
    public:   

        NObjL3Profile(Domain::NamedObject *pParent, 
			const Domain::ObjectName &name,
			IeConstants::SideType side = IeConstants::NT);

        Q_INVOKABLE void SetDefault(IeConstants::SideType side);

        Q_PROPERTY(QString Side READ Side WRITE Side);
		QString Side() const;
        Q_PROPERTY(bool StatActive READ m_statActive WRITE m_statActive);  

    // Option:
        Q_PROPERTY(bool SendConnectAck READ m_SendConnectAck WRITE m_SendConnectAck);
        Q_PROPERTY(QString Location READ getLocation WRITE setLocation );
        Q_PROPERTY(bool SendCallingIfExist READ m_SendCallingIfExist WRITE m_SendCallingIfExist);
        Q_PROPERTY(bool SendProgInd READ m_SendProgInd WRITE m_SendProgInd);        
        Q_PROPERTY(bool AssignBChannalsIntfId READ m_AssignBChannalsIntfId WRITE m_AssignBChannalsIntfId); 
        Q_PROPERTY(bool AutoActivate READ m_AutoReActivation WRITE m_AutoReActivation);

    // DssTimersProf:                
        Q_PROPERTY(QString T301 READ getT301);
        QString getT301() const {return m_timers.T301.ToString();}
        Q_INVOKABLE void T301(int period, int repeat = 1) {m_timers.T301 = DssTimerValue(period, repeat);  }

        Q_PROPERTY(QString T302 READ getT302);
        QString getT302() const {return m_timers.T302.ToString();}
        Q_INVOKABLE void T302(int period, int repeat = 1) { m_timers.T301 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T303 READ getT303);
        QString getT303() const {return m_timers.T303.ToString();}
        Q_INVOKABLE void T303(int period, int repeat = 1) { m_timers.T303 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T304 READ getT304);
        QString getT304() const {return m_timers.T304.ToString();}
        Q_INVOKABLE void T304(int period, int repeat = 1) { m_timers.T304 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T305 READ getT305);
        QString getT305() const {return m_timers.T305.ToString();}
        Q_INVOKABLE void T305(int period, int repeat = 1) { m_timers.T305 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T306 READ getT306);
        QString getT306() const {return m_timers.T306.ToString();}
        Q_INVOKABLE void T306(int period, int repeat = 1) { m_timers.T306 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T307 READ getT307);
        QString getT307() const {return m_timers.T307.ToString();}
        Q_INVOKABLE void T307(int period, int repeat = 1) { m_timers.T307 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T308 READ getT308);
        QString getT308() const {return m_timers.T308.ToString();}
        Q_INVOKABLE void T308(int period, int repeat = 1) { m_timers.T308 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T309 READ getT309);
        QString getT309() const {return m_timers.T309.ToString();}
        Q_INVOKABLE void T309(int period, int repeat = 1) { m_timers.T309 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T310 READ getT310);
        QString getT310() const {return m_timers.T310.ToString();}
        Q_INVOKABLE void T310(int period, int repeat = 1) { m_timers.T310 = DssTimerValue(period, repeat); }             

        Q_PROPERTY(QString T312 READ getT312);
        QString getT312() const {return m_timers.T312.ToString();}
        Q_INVOKABLE void T312(int period, int repeat = 1) { m_timers.T312 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T313 READ getT313);
        QString getT313() const {return m_timers.T313.ToString();}
        Q_INVOKABLE void T313(int period, int repeat = 1) { m_timers.T313 = DssTimerValue(period, repeat); }


        Q_PROPERTY(QString T314 READ getT314);
        QString getT314() const {return m_timers.T314.ToString();}
        Q_INVOKABLE void T314(int period, int repeat = 1) { m_timers.T314 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T316 READ getT316);
        QString getT316() const {return m_timers.T316.ToString();}
        Q_INVOKABLE void T316(int period, int repeat = 1) { m_timers.T316 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T317 READ getT317);
        QString getT317() const {return m_timers.T317.ToString();}
        Q_INVOKABLE void T317(int period, int repeat = 1) { m_timers.T317 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T318 READ getT318);
        QString getT318() const {return m_timers.T318.ToString();}
        Q_INVOKABLE void T318(int period, int repeat = 1) { m_timers.T318 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T319 READ getT319);
        QString getT319() const {return m_timers.T319.ToString();}
        Q_INVOKABLE void T319(int period, int repeat = 1) { m_timers.T319 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T320 READ getT320);
        QString getT320() const {return m_timers.T320.ToString();}
        Q_INVOKABLE void T320(int period, int repeat = 1) { m_timers.T320 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T321 READ getT321);
        QString getT321() const {return m_timers.T321.ToString();}
        Q_INVOKABLE void T321(int period, int repeat = 1) { m_timers.T321 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString T322 READ getT322);
        QString getT322() const {return m_timers.T322.ToString();}
        Q_INVOKABLE void T322(int period, int repeat = 1) { m_timers.T322 = DssTimerValue(period, repeat); }        

        Q_PROPERTY(QString TReActivateReqL3 READ getTReActivateReqL3);
        QString getTReActivateReqL3() const {return m_timers.tReActivateReqL3.ToString();}
        Q_INVOKABLE void TReActivateReqL3(int period, int repeat = 1) 
        { m_timers.tReActivateReqL3 = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString TWaitFreeWinInd READ getTWaitFreeWinInd);
        QString getTWaitFreeWinInd() const {return m_timers.tWaitFreeWinInd.ToString();}
        Q_INVOKABLE void TWaitFreeWinInd(int period, int repeat = 1) 
        { m_timers.tWaitFreeWinInd = DssTimerValue(period, repeat); }

        Q_PROPERTY(QString TProcessErrorF READ getTProcessErrorF);
        QString getTProcessErrorF() const {return m_timers.tProcessErrorF.ToString();}
        Q_INVOKABLE void TProcessErrorF(int period, int repeat = 1) 
        { m_timers.tProcessErrorF = DssTimerValue(period, repeat); }                                     

    // user interface:
    public:
		boost::shared_ptr<L3Profile> GenerateProfile(Utils::SafeRef<IDssToGroup> intfGroup,
			const std::string &nameDss1);

    // RW methods:
    private:
        QString getLocation() const;
        void setLocation(QString enumVal);        
                     
		void Side(const QString &side);

    private:
		IeConstants::SideType m_sideType;
		bool m_statActive;
		DssTimersProf m_timers;

        NObjDssTraceOption* m_pTraceOptions;


    };
} // namespace ISDN

#endif

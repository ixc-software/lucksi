#pragma once

#include "stdafx.h"
#include "CallMeetingState.h"

#include "CallMeetingProfile.h"
#include "CallMeetingStat.h"
#include "CountryFilter.h"
#include "Domain/NamedObject.h"
#include "Domain/DomainClass.h"
#include "Utils/HostInf.h"
#include "Utils/ManagedList.h"
#include "Utils/WeakRef.h"
#include "iSip/ISipAccessPoint.h"

namespace CallMeeting
{
	class CallingIpFilter;
	class CallMeetingCustomer;

	class NObjCallMeeting : public Domain::NamedObject,
		public iSip::ISipAccessPointEvents
	{
		Q_OBJECT;

		typedef NObjCallMeeting T;

	public:

		NObjCallMeeting(Domain::IDomain *pDomain, const Domain::ObjectName &name); 
		~NObjCallMeeting();
		
		Q_PROPERTY(bool Enabled READ Enabled);
		Q_INVOKABLE void Start();
		Q_INVOKABLE void StartTest(int localRtpPort);
		Q_INVOKABLE void Stop();

		Q_PROPERTY(QString SipPoint READ m_sipAccessPointName WRITE SipPoint);

		Q_PROPERTY(bool UseCustomerRules READ m_useCustomerRules WRITE UseCustomerRules);
		Q_PROPERTY(bool UseRedirect READ UseRedirect WRITE UseRedirect);
		Q_PROPERTY(int WaitConnectionTimeout READ WaitConnectionTimeout WRITE WaitConnectionTimeout);
		Q_PROPERTY(QString TryingTimeout READ TryingTimeout WRITE TryingTimeout);
		Q_PROPERTY(QString AlertingTimeout READ AlertingTimeout WRITE AlertingTimeout);		
		Q_PROPERTY(QString ConnectToOtherFilterTimeout READ ConnectToOtherFilterTimeout WRITE ConnectToOtherFilterTimeout);		
		Q_PROPERTY(int MaxTalkDurationMin READ MaxTalkDurationMin WRITE MaxTalkDurationMin);		
		Q_PROPERTY(bool ConnectImmediately READ ConnectImmediately WRITE ConnectImmediately);
		Q_PROPERTY(bool RejectIfDstBusy READ RejectIfDstBusy WRITE RejectIfDstBusy);
		Q_PROPERTY(int CheckRuleTimeoutMin READ m_checkRuleTimeoutMin WRITE m_checkRuleTimeoutMin);

		Q_PROPERTY(int StateStoreUpdateTimeoutMin READ m_storeTimeout WRITE StateStoreTimeoutMin);
		Q_PROPERTY(QString StateStoreFileName READ GetStoreFilename);

		Q_INVOKABLE void GetCallingIpFilter(DRI::ICmdOutput *pOutput); 

		Q_INVOKABLE void AddCallingIpFilter(const QString &ip); 
		Q_INVOKABLE void DelCallingIpFilter(const QString &ip); 

		Q_INVOKABLE void LoadCountriesTable(const QString &fileName); 
		Q_INVOKABLE void SaveCountriesTable(const QString &fileName); 
		Q_INVOKABLE void ClearCountriesTable();

		Q_INVOKABLE void AddCustomer(const QString &customerName, const QString &number); 
		Q_INVOKABLE void DelCustomer(const QString &customerName); 
		Q_INVOKABLE void EnableCustomerTrace(const QString &customerName, bool isEnabled); 

		Q_INVOKABLE void LoadCustomersTable(const QString &fileName);
		Q_INVOKABLE void SaveCustomersTable(const QString &fileName);
		Q_INVOKABLE void ClearCustomersTable();
		


		Q_INVOKABLE void SetRule(const QString &customer, const QString &country, int density, int percent);
		Q_INVOKABLE void LoadRulesTable(DRI::IAsyncCmd *pAsyncCmd, const QString &fileName);
		Q_INVOKABLE void SaveRulesTable(DRI::IAsyncCmd *pAsyncCmd, const QString &fileName);
		Q_INVOKABLE void ClearRulesTable();

		Q_INVOKABLE void Stat(DRI::ICmdOutput *pOutput);
		Q_INVOKABLE void RestartStat();

		Q_INVOKABLE void StatByCountry(DRI::ICmdOutput *pOutput, const QString &country = "");
		Q_INVOKABLE void StatByCustomer(DRI::ICmdOutput *pOutput, const QString &customer = "");
		Q_INVOKABLE void StatByRule(DRI::ICmdOutput *pOutput, const QString &customer, const QString &country);

		Q_INVOKABLE void StatIpFilter(DRI::ICmdOutput *pOutput, const QString &ip = "");

		const CallMeetingProfile &Profile() const
		{
			return m_profile;
		}
		
		CountryFilter::QueuedCall *AllocCallReadyForOtherFilter(const CountryFilter &ownerCall);

	// iSip::ISipAccessPointEvents
	private:

		void OnIncomingParticipant(const iSip::ISipAccessPoint *src,
			Utils::SafeRef<iSip::IRemoteParticipant> newParticipant,
			const resip::SipMessage &msg,
			bool autoAnswer, 
			const iSip::ConversationProfile &conversationProfile);

		void OnRequestOutgoingParticipant(const iSip::ISipAccessPoint *src,
			Utils::SafeRef<iSip::IRemoteParticipant> newParticipant, 
			const resip::SipMessage &msg, 
			const iSip::ConversationProfile &conversationProfile);

	// property
	private: 
		void SipPoint(const QString &val);
		void UseCustomerRules(bool val);
		bool UseRedirect() const	{ return m_profile.UseRedirect(); }
		void UseRedirect(bool val)	{ m_profile.UseRedirect(val); }
		void TryingTimeout(const QString &val);
		QString TryingTimeout() const;
		void AlertingTimeout(const QString &val);
		QString AlertingTimeout() const;
		void ConnectToOtherFilterTimeout(const QString &val);
		QString ConnectToOtherFilterTimeout() const;
		void WaitConnectionTimeout(int val);
		int WaitConnectionTimeout() const;
		void MaxTalkDurationMin(int val);
		int MaxTalkDurationMin() const;
		void ConnectImmediately(bool val);
		bool ConnectImmediately() const;
		void RejectIfDstBusy(bool val);
		bool RejectIfDstBusy() const;\

	private:
		boost::shared_ptr<CountryFilter> SearchCountryFilter(const QString &country);
		boost::shared_ptr<CallMeetingCustomer>
			SearchCustomer(const QString &customer);
		boost::shared_ptr<CallMeetingCustomer>
			SearchCustomerByNumber(const std::string &number);
		int  SearchIpFilter(const QString &ip);
		
		void ShortStat(DRI::ICmdOutput *pOutput, 
			boost::function<void (Utils::TableOutput &) > statList);		
		
		void ShortStatByCountry(DRI::ICmdOutput *pOutput);
		void ShortStatByCustomer(DRI::ICmdOutput *pOutput);
		void ShortStatByIp(DRI::ICmdOutput *pOutput);
		void OutputTable(DRI::ICmdOutput *pOutput, const Utils::TableOutput &table);
		bool Enabled() const;
		void OnPulse(iCore::MsgTimer *pT);
		void OnPulseRule(iCore::MsgTimer *pT);
		void OnStore(iCore::MsgTimer *pT);

		void LoadFile(const QString &fileName, boost::function<void (const QStringList&)> parser);

		void LoadFile(const QString &par, QStringList &out);
		void SaveFile(const QStringList &data, const QString &fileName);
		
		void ParseCountryLine(const QStringList &line);
		void AddCountry(const QString &countryName, const QString &code);

		void ParseCustomerLine(const QStringList &line);

		typedef QList<boost::shared_ptr<CallMeetingCustomer> > CustomersSubSet;

		boost::shared_ptr<CustomersSubSet> CustomersList(const QStringList &line);

		void CheckRuleLine(const QStringList &line, 
			const CustomersSubSet &customers);

		void ProcessRuleLine(const QStringList &line, 
			CustomersSubSet &customers);

		struct RulesLoader;
		struct RulesSaver;

		void OnTestRuleLine(boost::shared_ptr<RulesLoader> );
		void OnProcessRuleLine(boost::shared_ptr<RulesLoader> );
		void OnSaveRuleLine(boost::shared_ptr<RulesSaver> );

		void StateStoreTimeoutMin(int timeoutMin);
		QString GetStoreFilename() const;
		void SaveState(CallMeetingState::CustomerList &customers) const;
		void UpdateSate(const CallMeetingState::CustomerList &customers);

	private:        
		
		Utils::WeakRefHost m_selfHost;

		bool m_useCustomerRules;
		bool m_useRedirect;

		iCore::MsgTimer m_pulseTimer;
		iCore::MsgTimer m_rulesTimer;
		iCore::MsgTimer m_storeTimer;

		int m_checkRuleTimeoutMin;

		QString m_sipAccessPointName;

		Utils::SafeRef<iSip::ISipAccessPoint> m_sipAccessPoint;

		bool m_isEnabled;

		class Statistic : boost::noncopyable
		{
		public:
			Statistic();
			void ToString(QStringList &out) const;
			void Restart();
			void IncUnknownCountry();
			void IncUnknownIp();
			void IncUnknownUserA();
			CallMeetingStat &Base();

		private:
			CallMeetingStat m_base;
			int m_unknownCountry;
			int m_unknownIp;
			int m_unknownUserA;
		};

		Statistic m_stat;
		CallMeetingProfile m_profile;
		typedef QMap<QString, boost::shared_ptr<CountryFilter> > ListCountryFilter;
		ListCountryFilter m_countryFilters;

		typedef QMap<std::string, boost::shared_ptr<CallMeetingCustomer> > ListCustomers;
		ListCustomers m_customers;

		typedef Utils::ManagedList<CallingIpFilter> ListIpFilter;
		ListIpFilter m_ipFilters;

		int m_countRejCallsByWrongSerial;
        QRegExp m_regexpNotDigit;

		int m_storeTimeout;
	};
};



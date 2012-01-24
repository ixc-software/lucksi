#include "stdafx.h"
#include "Sip/undeferrno.h"
#include "NObjCallMeeting.h"
#include "CallMeetingRule.h"
#include "CountryFilter.h"
#include "CallingIpFilter.h"
#include "QueuedMeetingCall.h"
#include "CallMeetingStat.h"
#include "CallMeetingCustomer.h"
#include "Utils/HostInf.h"
#include "Utils/ExeName.h"
#include "Pcid/ProtSerial.h"
#include "Utils/OutputFormatters.h"
#include "Utils/GetDefLocalHost.h"
#include "resip/stack/SipMessage.hxx"
#include "iSip/NObjSipAccessPoint.h"

#include "iSip/SipUtils.h"
#include "iSip/SipMessageHelper.h"

namespace
{
	const int CStateTimeoutMin = 5;
	const int GPulseTimeout = 100;
	const int GCheckRulesTimeout = 5;
	const bool CCheckSerialOn = true;	
	const bool CAssertIfCheckSerialWrong = false;	
	const int CCheckCallSize = 5;
	const int CRejCallsWrongSerial = 1000;
	const char *CTableSplitter = "\t";
	const char *CRuleSplitter = "|";

	bool CheckPcid()
	{
		if (!CCheckSerialOn) return true;

		static int count = 0;		
		bool res = true;
		if (count < CCheckCallSize) 
			++count;
		else
		{
			res = Pcid::ProtSerial::CheckSerialFile();
			if (CAssertIfCheckSerialWrong) ESS_ASSERT(res);
			count = 0;
		}
		return res;
	}

	QString FileName(const QString &fileName)
	{
		QFileInfo fileInfo(fileName);
		if (!fileInfo.isAbsolute())
		{ 
			fileInfo = Utils::ExeName::GetExeDir().c_str() + fileName;
		}
		return fileInfo.absoluteFilePath();
	}

	Platform::dword GetRandomValue(const Utils::ValueRange &range, Utils::Random &random)
	{
		return random.NextInRange(range.From(), range.To());
	}
}

namespace CallMeeting
{
	using namespace resip;

	NObjCallMeeting::NObjCallMeeting(Domain::IDomain *pDomain, const Domain::ObjectName &name) : 
		Domain::NamedObject(pDomain, name),
		m_pulseTimer(this, &T::OnPulse),
		m_rulesTimer(this, &T::OnPulseRule),
		m_storeTimer(this, &T::OnStore),
        m_isEnabled(false),
		m_countRejCallsByWrongSerial(0),
        m_regexpNotDigit("(\\D)"),
		m_storeTimeout(0)
	{
		m_useCustomerRules = true;

		m_pulseTimer.Start(GPulseTimeout, true);
		m_checkRuleTimeoutMin = GCheckRulesTimeout;
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Created." << iLogW::EndRecord;
	}

	// -------------------------------------------------------------------------------

	NObjCallMeeting::~NObjCallMeeting()
	{
		if (IsLogActive(InfoTag)) Log(InfoTag) << "Deleted." << iLogW::EndRecord;
	}


	// -------------------------------------------------------------------------------

	void NObjCallMeeting::Start()
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");

		{
			std::string error;
			if (!m_profile.Validate(error)) ThrowRuntimeException(error);
		}

		if (m_ipFilters.IsEmpty())
		{
			ThrowRuntimeException("Ip filter is empty. Cant start application.");
		}

		m_isEnabled = true;
		m_stat.Restart();

		if (m_storeTimeout != 0)
		{
			CallMeetingState::CustomerList customers;
			if (CallMeetingState::Read(customers))
			{
				UpdateSate(customers);
			}
		}

		m_rulesTimer.Start(m_checkRuleTimeoutMin * 60 * 1000, true);

		if (m_storeTimeout != 0) m_storeTimer.Start(m_storeTimeout, true);

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Started." << iLogW::EndRecord;		
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::StartTest(int localRtpPort)
	{
		Start();
		m_profile.FakeRtp(Utils::GetDefLocalHostInf(localRtpPort));
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::Stop()
	{
		if (!m_isEnabled) ThrowRuntimeException("Object is already stopped.");

		m_isEnabled = false;

		m_rulesTimer.Stop();

		if (IsLogActive(InfoTag)) Log(InfoTag) << "Stopped." << iLogW::EndRecord;		
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::GetCallingIpFilter(DRI::ICmdOutput *pOutput)
	{
		QString res("[ ");
		for (int i = 0; i < m_ipFilters.Size(); ++i)
		{
			res += m_ipFilters[i]->Ip + "; ";
		}
		res += "]";
		pOutput->Add(res);
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::AddCallingIpFilter(const QString &ip)
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");		

		if (SearchIpFilter(ip) != -1) ThrowRuntimeException(QString("Filter '%1' already exists").arg(ip));

		{
			std::string err;
			Utils::HostInf res;
			if (!Utils::HostInf::FromString(ip.toStdString(), 
				res,
				Utils::HostInf::HostOnly,
				&err))
			{
				ThrowRuntimeException(QString("Wrong value. %1").arg(err.c_str()));
			}
		}
		m_ipFilters.Add(new CallingIpFilter(ip));
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::DelCallingIpFilter(const QString &ip)
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");		

		int pos = SearchIpFilter(ip);
		if (pos == -1) ThrowRuntimeException(QString("This ip filter doesn't exist: %1").arg(ip));

		m_ipFilters.Delete(pos);
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::LoadCountriesTable(const QString &fileName)
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");

		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		if (!m_countryFilters.isEmpty()) ThrowRuntimeException("Countries table isn't empty.");
		
		if (!m_customers.isEmpty()) ThrowRuntimeException("Customers table isn't empty.");

		LoadFile(fileName, boost::bind(&T::ParseCountryLine, this, _1));
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::SaveCountriesTable(const QString &fileName)
	{
		if (m_countryFilters.isEmpty()) ThrowRuntimeException("Countries table is empty.");

		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		QStringList countries;

		for(ListCountryFilter::const_iterator i = m_countryFilters.begin(); 
			i != m_countryFilters.end(); ++i)
		{
			for(int c = 0; c < i.value()->Codes().size(); ++c)
			{
				countries.append(QString("%1\t%2").
					arg(i.value()->Country()).
					arg(i.value()->Codes()[c]));
			}
		}
		SaveFile(countries, fileName);
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::ClearCountriesTable()
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");

		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		if (!m_customers.isEmpty()) ThrowRuntimeException("Customers table isn't empty.");

		if (!m_countryFilters.isEmpty())	m_countryFilters.clear();
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::AddCustomer(const QString &customer, const QString &number)
	{
		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		if (customer.isEmpty() || customer.count(" ") == customer.size()) 
		{
			ThrowRuntimeException("Customer is empty");
		}
		if (number.isEmpty() || number.count(" ") == number.size()) 
		{
			ThrowRuntimeException("Number is empty");
		}

		if (SearchCustomer(customer) != 0) 
		{
			ThrowRuntimeException(QString("This customer already exist. Name: %1, number: %2").
				arg(customer).arg(number));
		}
		boost::shared_ptr<CallMeetingCustomer> ptr(new CallMeetingCustomer(
			Log().LogCreator(), 
			customer, number));
		m_customers.insert(number.toStdString(), ptr);
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::DelCustomer(const QString &customerName)
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");		

		if (customerName.isEmpty()) ThrowRuntimeException("Customer is empty");
		
		for(ListCustomers::iterator i = m_customers.begin(); 
			i != m_customers.end(); ++i)
		{
			if (i.value()->Name() == customerName) 
			{
				m_customers.erase(i);
				return;
			}
		}

		ThrowRuntimeException(QString("Customer '%1' not found.").
			arg(customerName));
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::EnableCustomerTrace(const QString &customerName, bool isEnabled)
	{
		if (customerName.isEmpty()) ThrowRuntimeException("Customer is empty");

		boost::shared_ptr<CallMeetingCustomer> customer = 
			SearchCustomer(customerName);

		if (customer == 0)
		{
			ThrowRuntimeException(QString("Customer '%1' not found.").
				arg(customerName));
		}

		customer->EnableTrace(isEnabled);

	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::LoadCustomersTable(const QString &fileName)
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");

		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		if (!m_customers.isEmpty()) ThrowRuntimeException("Customers table isn't empty.");

		LoadFile(fileName, boost::bind(&T::ParseCustomerLine, this, _1));		
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::SaveCustomersTable(const QString &fileName)
	{
		if (m_customers.isEmpty()) ThrowRuntimeException("Customers table is empty.");

		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		QStringList customers;
		for(ListCustomers::const_iterator i = m_customers.begin(); 
			i != m_customers.end(); ++i)
		{
			customers.append(QString("%1\t%2").
				arg(i.value()->Name()).
				arg(i.value()->Number().c_str()));
		}
		SaveFile(customers, fileName);
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::ClearCustomersTable()
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");

		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		if (!m_customers.isEmpty()) m_customers.clear();
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::SetRule(const QString &customer, const QString &country, int density, int percent)
	{
		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		if (SearchCountryFilter(country) == 0) 
		{
			ThrowRuntimeException(QString("Country '%1' not found").arg(country));
		}
		boost::shared_ptr<CallMeetingCustomer> pCustomer = SearchCustomer(customer);
		if (pCustomer == 0) ThrowRuntimeException(QString("Customer '%1' not found").arg(customer));
		pCustomer->SetRuleParameters(country, CallMeetingRulePar(density, percent));
	}

	// ---------------------------------------------------------------------

	struct NObjCallMeeting::RulesLoader : boost::noncopyable
	{
	public:
		RulesLoader(boost::shared_ptr<CustomersSubSet> customers,
			const QStringList &rules) : 
			m_current(0),
			m_customers(customers),
			m_rules(rules)
		{}

		bool IsComplete() const
		{
			return m_current >= m_rules.size();
		}

		QStringList ProcessLine()
		{
			ESS_ASSERT(!IsComplete());
			return m_rules.at(m_current++).split(CTableSplitter);
		}

		boost::shared_ptr<CustomersSubSet> Customers()
		{
			return m_customers;
		}
		void Reset()
		{
			m_current = 0;
		}
	private:
		int m_current;
		boost::shared_ptr<CustomersSubSet> m_customers;
		QStringList m_rules;

	};

	// ---------------------------------------------------------------------

	void NObjCallMeeting::LoadRulesTable(DRI::IAsyncCmd *pAsyncCmd, const QString &fileName)
	{
		if (m_countryFilters.isEmpty()) ThrowRuntimeException("Countries table is empty.");

		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		if (m_customers.isEmpty()) ThrowRuntimeException("Customers table is empty.");

		QStringList rules;
		LoadFile(fileName, rules);

		if (rules.size() < 2) ThrowRuntimeException("Rules table is too short.");

		AsyncBegin(pAsyncCmd);	

		boost::shared_ptr<CustomersSubSet> customers = 
			CustomersList(rules.at(0).split(CTableSplitter));
		rules.pop_front();

		boost::shared_ptr<RulesLoader> com(new RulesLoader(customers, rules));

		PutMsg(this, &T::OnTestRuleLine, com);
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::OnTestRuleLine(boost::shared_ptr<RulesLoader> com)
	{
		if (!AsyncActive()) return;

		CheckRuleLine(com->ProcessLine(), *com->Customers());

		if (!com->IsComplete())	
			PutMsg(this, &T::OnTestRuleLine, com);
		else
		{
			com->Reset();
			PutMsg(this, &T::OnProcessRuleLine, com);
		}
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::OnProcessRuleLine(boost::shared_ptr<RulesLoader> com)
	{
		if (!AsyncActive()) return;

		ProcessRuleLine(com->ProcessLine(), *com->Customers());
		if (com->IsComplete())	
			AsyncComplete(true);
		else
			PutMsg(this, &T::OnProcessRuleLine, com);
	}

	// ---------------------------------------------------------------------

	struct NObjCallMeeting::RulesSaver
	{
		QString FileName;
		QString Country;
		QStringList Rules;
	};

	void NObjCallMeeting::SaveRulesTable(DRI::IAsyncCmd *pAsyncCmd, const QString &fileName)
	{
		if (m_countryFilters.isEmpty()) ThrowRuntimeException("Countries table is empty.");

		if (!m_useCustomerRules) ThrowRuntimeException("It is simple mode.");

		if (m_customers.isEmpty()) ThrowRuntimeException("Customers table is empty.");
	
		AsyncBegin(pAsyncCmd);	

		boost::shared_ptr<RulesSaver> com(new RulesSaver());
		com->FileName = fileName;

		QString customers(" ");
		for(ListCustomers::const_iterator i = m_customers.begin(); 
			i != m_customers.end(); ++i)
		{
			customers += CTableSplitter;
			customers += i.value()->Name();
		}
		com->Rules << customers;
		com->Country = m_countryFilters.begin().value()->Country();
		PutMsg(this, &T::OnSaveRuleLine, com);
	}

	void NObjCallMeeting::OnSaveRuleLine(boost::shared_ptr<RulesSaver> com)
	{
		ListCountryFilter::const_iterator country = m_countryFilters.find(com->Country); 
		ESS_ASSERT(country != m_countryFilters.end());

		QString rule(com->Country);

		for(ListCustomers::const_iterator i = m_customers.begin(); 
			i != m_customers.end(); ++i)
		{
			rule += CTableSplitter;
			rule += i.value()->RuleParameters(country.value()->Country()).SaveRule(CRuleSplitter);
		}
		com->Rules << rule;

		if (++country != m_countryFilters.end())
		{
			com->Country = country.value()->Country();
			PutMsg(this, &T::OnSaveRuleLine, com);		
		}
		else
		{
			SaveFile(com->Rules, com->FileName);
			AsyncComplete(true);
		}
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::StateStoreTimeoutMin(int timeout)
	{
		if (timeout == 0)
		{
			m_storeTimeout = 0;
			m_storeTimer.Stop();
			return;
		}

		if (timeout < CStateTimeoutMin) 
		{
			ThrowRuntimeException("Timeout should be more then " + Utils::IntToString(CStateTimeoutMin) + " min");
		}

		m_storeTimeout = timeout * 1000 * 60;

		if (m_isEnabled) 
		{
			m_storeTimer.Stop();
			m_storeTimer.Start(m_storeTimeout, true);
		}
	}

	// ---------------------------------------------------------------------

	QString NObjCallMeeting::GetStoreFilename() const
	{
		return CallMeetingState::StoreFilename();
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::SaveState(CallMeetingState::CustomerList &customers) const
	{
		for(ListCustomers::const_iterator i = m_customers.begin();
			i != m_customers.end(); ++i)
		{
			CallMeetingState::Customer customer(i.value()->SaveState());

			if (!customer.Rules.isEmpty()) customers.push_back(customer);
		}
	}


	// -------------------------------------------------------------------------------

	void NObjCallMeeting::UpdateSate(const CallMeetingState::CustomerList &customers)
	{
		for (CallMeetingState::CustomerList::const_iterator i = customers.begin();
			i != customers.end(); ++i)
		{
			boost::shared_ptr<CallMeetingCustomer> customer = 
				SearchCustomer(i->Name);

			if (customer != 0) customer->UpdateSate(*i);
		}
	}

	// ---------------------------------------------------------------------

	void NObjCallMeeting::ClearRulesTable()
	{
		for(ListCustomers::iterator i = m_customers.begin(); 
			i != m_customers.end(); ++i)
		{
			i.value()->ClearRules();
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::Stat(DRI::ICmdOutput *pOutput)
	{
		QStringList out;
		m_stat.ToString(out);
		pOutput->Add(out);
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::RestartStat()
	{
		m_stat.Restart();

		for(ListCountryFilter::iterator i = m_countryFilters.begin(); 
			i != m_countryFilters.end();
			++i)
		{
			i.value()->RestartStat();
		}
		for (int i = 0; i < m_ipFilters.Size(); ++i)
		{
			m_ipFilters[i]->Stat.Restart();
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::StatByCountry(DRI::ICmdOutput *pOutput, const QString &country)
	{
		if (country.isEmpty())
			ShortStatByCountry(pOutput);
		else
		{
			boost::shared_ptr<CountryFilter> pCountry = SearchCountryFilter(country);

			if (pCountry == 0) ThrowRuntimeException(QString("Country '%1' not found").arg(country));

			QStringList out;
			pCountry->StatAll(out);
			pOutput->Add(out);
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::StatByCustomer(DRI::ICmdOutput *pOutput, const QString &customer)
	{
		if (customer.isEmpty())
			ShortStatByCustomer(pOutput);
		else
		{
			boost::shared_ptr<CallMeetingCustomer> pCustomer = SearchCustomer(customer);

			if (pCustomer == 0) ThrowRuntimeException(QString("Customer '%1' not found").arg(customer));

			QStringList out;
			pCustomer->StatAll(out);
			pOutput->Add(out);
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::StatByRule(DRI::ICmdOutput *pOutput, const QString &customer, const QString &country)
	{
		if (SearchCountryFilter(country) == 0) ThrowRuntimeException(QString("Country '%1' not found").arg(country));

		boost::shared_ptr<CallMeetingCustomer> pCustomer = 
			SearchCustomer(customer);

		if (pCustomer == 0) ThrowRuntimeException(QString("Customer '%1' not found").arg(customer));

		QStringList out;
		pCustomer->StatByRule(out, country, m_checkRuleTimeoutMin * 60);
		pOutput->Add(out);
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::StatIpFilter(DRI::ICmdOutput *pOutput, const QString &ip)
	{
		if (ip.isEmpty())
			ShortStatByIp(pOutput);
		else
		{
			int i = SearchIpFilter(ip);

			if (i == -1) ThrowRuntimeException(QString("Cant find filter: %1").arg(ip));

			QStringList out;
			m_ipFilters[i]->StatAll(out);
			pOutput->Add(out);
		}
	}

	// ---------------------------------------------------------------------

	CountryFilter::QueuedCall *NObjCallMeeting::AllocCallReadyForOtherFilter(const CountryFilter &ownerCall)
	{
		for(ListCountryFilter::iterator i = m_countryFilters.begin(); 
			i != m_countryFilters.end();
			++i)
		{
			if (i.value().get() == &ownerCall) continue;

			CountryFilter::QueuedCall *call = 
				i.value()->AllocCallReadyForOtherFilter();
			if (call != 0) return call;
		}
		return 0;
	}

	// -------------------------------------------------------------------------------------
	// ISipCallManagerEvents

	void NObjCallMeeting::OnIncomingParticipant(const iSip::ISipAccessPoint *src,
		Utils::SafeRef<iSip::IRemoteParticipant> callSip,
		const resip::SipMessage &msg,
		bool autoAnswer, 
		const iSip::ConversationProfile &conversationProfile)
	{
		ESS_ASSERT(m_sipAccessPoint.IsEqualIntf(src));

		boost::shared_ptr<CallMeetingStatList> stat(new CallMeetingStatList);
		stat->Push(m_stat.Base());

		if (m_countRejCallsByWrongSerial != 0)
		{
			if (m_countRejCallsByWrongSerial < CRejCallsWrongSerial) 
			{
				m_countRejCallsByWrongSerial++;
				if (IsLogActive(InfoTag)) 
				{
					Log(InfoTag) << "Reject call. Cause: wrong serial number. " 
						<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
						<< iLogW::EndRecord;
				}
				callSip->DestroyParticipant(403); // 403 - forbidden
				return;
			}

			m_countRejCallsByWrongSerial = 0;			
		}

		if (!CheckPcid()) m_countRejCallsByWrongSerial++;

		iSip::SipMessageHelper msgHelper(msg);

		QString calledAddr(msgHelper.To().uri().user().c_str());

		QString callingAddr(msgHelper.From().uri().user().c_str());

		int ipPos = SearchIpFilter(msgHelper.From().uri().host().c_str());
		if (ipPos == -1)
		{
			if (IsLogActive(InfoTag))
			{
				Log(InfoTag) << "Calling ip '" 
					<< callingAddr.toStdString()
					<< "' isn't filtered. " 
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}
			m_stat.IncUnknownIp();
			callSip->DestroyParticipant(406);
			return;
		}	

		stat->Push(m_ipFilters[ipPos]->Stat);

		CountryFilter *country = 0;

		if (!m_useCustomerRules)
		{
			if (!m_countryFilters.empty()) country = m_countryFilters.begin().value().get();
		}
		else
		{
			for(ListCountryFilter::iterator i = m_countryFilters.begin(); 
				i != m_countryFilters.end();
				++i)
			{
				if (i.value()->AcceptNewCall(calledAddr, stat))
				{
					country = i.value().get();
					break;
				}
			}
		}

		if (country == 0)
		{
			if (IsLogActive(InfoTag)) 
			{
				Log(InfoTag) << "Country code not found. " 
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}
			m_stat.IncUnknownCountry();
			callSip->DestroyParticipant(406);
			return;
		}

		if (!m_useCustomerRules)
		{
			if (IsLogActive(InfoTag))
			{
				Log(InfoTag) << "Simple mode. Call accepted." 
					<< " Sip call: " 
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}
			country->IncomingCall(callSip, msg, stat);
			return;
		}

		boost::shared_ptr<CallMeetingCustomer> customer = 
			SearchCustomerByNumber(callingAddr.toStdString());

		if (customer == 0)
		{
			if (IsLogActive(InfoTag)) 
			{
				Log(InfoTag) << "Customer not found. " 
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}

			m_stat.IncUnknownUserA();
			callSip->DestroyParticipant(406);
			return;
		}

		CallMeetingRule *rule = customer->RuleForNewCall(country->Country(), stat);
		if (rule == 0)		
		{
			if (IsLogActive(InfoTag))
			{
				Log(InfoTag) << "Call not accepted. Rule not defined. Sip call: " 
					<< "; Country: " << country->Country()
					<< "; Customer: " << customer->Name() 
					<< "; "
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}
			callSip->DestroyParticipant(406);
			return;
		}

		if (!rule->CheckIncomingCall(stat))		
		{
			if (IsLogActive(InfoTag))
			{
				Log(InfoTag) << "Call not accepted by rule.  Sip call: " 
					<< "; Country: " << country->Country()
					<< "; Customer: " << customer->Name() 
					<< "; "
					<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
					<< iLogW::EndRecord;
			}
			callSip->DestroyParticipant(406);
			return;
		}

		if (IsLogActive(InfoTag))
		{
			Log(InfoTag) << "Call accepted." 
				<< " Sip call: " 
				<< "; Country: " << country->Country()
				<< "; Customer: " << customer->Name() 
				<< "; Rule: " << rule->Brief()
				<< "; "
				<< iSip::SipUtils::ToString(iSip::SipUtils::Short, msg) 
				<< iLogW::EndRecord;
		}

		country->IncomingCall(callSip, msg, stat);
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::OnRequestOutgoingParticipant(const iSip::ISipAccessPoint *src,
		Utils::SafeRef<iSip::IRemoteParticipant> newParticipant, 
		const resip::SipMessage &msg, 
		const iSip::ConversationProfile &conversationProfile)
	{
		ESS_UNIMPLEMENTED;
	}

	// -------------------------------------------------------------------------------
	// property

	void NObjCallMeeting::SipPoint(const QString &val)
	{
		if (val.isEmpty())
		{
			m_sipAccessPointName = val;
			m_sipAccessPoint.Clear();
			return;
		}

		iSip::NObjSipAccessPoint *accessPoint = dynamic_cast<iSip::NObjSipAccessPoint*>(
			getDomain().FindFromRoot(Domain::ObjectName(val)));

		if (accessPoint == 0) ThrowRuntimeException("NObjSipUserRegistration named '" + val + "' not found");

		m_sipAccessPointName = val;

		m_sipAccessPoint = accessPoint->AccessPoint();

		m_sipAccessPoint->SetCallReceiver(Name().GetShortName().toStdString(),
			m_selfHost.Create<iSip::ISipAccessPointEvents&>(*this)) ;
	}

	// -------------------------------------------------------------------------------------

	void NObjCallMeeting::UseCustomerRules(bool val)
	{
		if (m_isEnabled) ThrowRuntimeException("Object is already started.");

		if (m_useCustomerRules == val) return;

		ClearRulesTable();
		ClearCustomersTable();
		ClearCountriesTable();

		if (!val) AddCountry("Simple mode", "");

		m_useCustomerRules = val;
	}

	// -------------------------------------------------------------------------------------

	void NObjCallMeeting::TryingTimeout(const QString &val)
	{
		m_profile.m_tryingTimeout = CallMeetingProfile::TimeoutRange(val.toStdString());
	}

	// -------------------------------------------------------------------------------

	QString NObjCallMeeting::TryingTimeout() const
	{
		return m_profile.m_tryingTimeout.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::AlertingTimeout(const QString &val)
	{
		m_profile.m_alertingTimeout = CallMeetingProfile::TimeoutRange(val.toStdString());
	}

	// -------------------------------------------------------------------------------

	QString NObjCallMeeting::AlertingTimeout() const
	{
		return m_profile.m_alertingTimeout.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::ConnectToOtherFilterTimeout(const QString &val)
	{
		m_profile.m_crossFilterConnectTimeout = CallMeetingProfile::TimeoutRange(val.toStdString());
	}

	// -------------------------------------------------------------------------------

	QString NObjCallMeeting::ConnectToOtherFilterTimeout() const
	{
		return m_profile.m_crossFilterConnectTimeout.ToString().c_str();
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::WaitConnectionTimeout(int val)
	{
		m_profile.WaitConnectionTimeout(val);
	}

	// -------------------------------------------------------------------------------

	int NObjCallMeeting::WaitConnectionTimeout() const
	{
		return m_profile.WaitConnectionTimeout();
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::MaxTalkDurationMin(int val)
	{
		m_profile.MaxTalkDurationMin(val);
	}

	// -------------------------------------------------------------------------------

	int NObjCallMeeting::MaxTalkDurationMin() const
	{
		return m_profile.MaxTalkDurationMin();
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::ConnectImmediately(bool val)
	{
		m_profile.ConnectImmediately(val);
	}

	// -------------------------------------------------------------------------------

	bool NObjCallMeeting::ConnectImmediately() const
	{
		return m_profile.ConnectImmediately();
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::RejectIfDstBusy(bool val)
	{
		m_profile.RejectIfDstBusy(val);
	}

	// -------------------------------------------------------------------------------

	bool NObjCallMeeting::RejectIfDstBusy() const
	{
		return m_profile.RejectIfDstBusy();
	}

	// -------------------------------------------------------------------------------

	boost::shared_ptr<CountryFilter>
		NObjCallMeeting::SearchCountryFilter(const QString &country)
	{
		ListCountryFilter::iterator i = m_countryFilters.find(country);

		return (i != m_countryFilters.end()) ? i.value() :
			boost::shared_ptr<CountryFilter>();
	}

	// -------------------------------------------------------------------------------

	boost::shared_ptr<CallMeetingCustomer>
		NObjCallMeeting::SearchCustomer(const QString &customer)
	{
		for(ListCustomers::iterator i = m_customers.begin(); 
			i != m_customers.end(); ++i)
		{
			if (i.value()->Name() == customer) return i.value();
		}
		return boost::shared_ptr<CallMeetingCustomer>();
	}

	// -------------------------------------------------------------------------------

	boost::shared_ptr<CallMeetingCustomer>
		NObjCallMeeting::SearchCustomerByNumber(const std::string &number)
	{
		ListCustomers::iterator i = m_customers.find(number);

		if (i == m_customers.end()) return boost::shared_ptr<CallMeetingCustomer>();
		
		ESS_ASSERT(i.value()->Number() == number);

		return i.value();
	}
	
	// -------------------------------------------------------------------------------

	int  NObjCallMeeting::SearchIpFilter(const QString &ip)
	{
		for (int i = 0; i < m_ipFilters.Size(); ++i)
		{
			if (m_ipFilters[i]->Ip == ip) return i;
		}
		return -1;
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::ShortStat(DRI::ICmdOutput *pOutput, 
		boost::function<void (Utils::TableOutput &) > statList)
	{
		if (m_stat.Base().TotalCalls() == 0)
		{
			pOutput->Add("Statistic is empty");
			return;
		}

		Utils::TableOutput table("  ", ' ');
		CallMeetingStat::HeadShortStat(table);

		statList(table);

		OutputTable(pOutput, table);
	}

	// -------------------------------------------------------------------------------

	template <class Map> void ShortStatMap(const Map &map, Utils::TableOutput &table)
	{
        for(typename Map::const_iterator i = map.begin(); i != map.end(); ++i)
		{
			i.value()->StatShort(table);
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::ShortStatByCountry(DRI::ICmdOutput *pOutput)
	{
		ShortStat(pOutput, boost::bind(ShortStatMap<ListCountryFilter>, m_countryFilters, _1));
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::ShortStatByCustomer(DRI::ICmdOutput *pOutput)
	{
		ShortStat(pOutput, boost::bind(ShortStatMap<ListCustomers>, m_customers, _1));
	}

	// -------------------------------------------------------------------------------

	template <class List> void ShortStatManagedList(const List &list, Utils::TableOutput &table)
	{
		for(int i = 0; i != list.Size(); ++i)
		{
			list[i]->StatShort(table);
		}
	}

	void NObjCallMeeting::ShortStatByIp(DRI::ICmdOutput *pOutput)
	{
		ShortStat(pOutput, boost::bind(ShortStatManagedList<ListIpFilter>, boost::ref(m_ipFilters), _1));
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::OutputTable(DRI::ICmdOutput *pOutput, const Utils::TableOutput &table)
	{
		QStringList out;
		m_stat.Base().TimerHeader(out);
		pOutput->Add(out);
		pOutput->Add(table.Get());
	}

	// -------------------------------------------------------------------------------

	bool NObjCallMeeting::Enabled() const
	{
		return m_isEnabled;
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::OnPulse(iCore::MsgTimer *pT)
	{
		for(ListCountryFilter::iterator i = m_countryFilters.begin(); 
			i != m_countryFilters.end();
			++i)
		{
			i.value()->Process();
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::OnPulseRule(iCore::MsgTimer *pT)
	{
		if (!m_isEnabled) return;

		for(ListCustomers::iterator i = m_customers.begin(); 
			i != m_customers.end(); ++i)
		{
			i.value()->RuleTimeoutExpired();
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::OnStore(iCore::MsgTimer *pT)
	{
		CallMeetingState::CustomerList customers;
		SaveState(customers);
		CallMeetingState::Write(customers);
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::LoadFile(const QString &fileName, 
		boost::function<void (const QStringList&)> lineParser)
	{
		QStringList lines;
		LoadFile(fileName, lines);
		for(int i = 0; i < lines.size(); ++i)
		{
			lineParser(lines[i].split(CTableSplitter));
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::LoadFile(const QString &fileName, QStringList &out)
	{
		QString absoluteFileName = FileName(fileName);
		QFile file(absoluteFileName);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
		{
			ThrowRuntimeException(QString("File '%1' not found").arg(absoluteFileName));
		}
		QTextStream stream(&file);
		while (!stream.atEnd())
		{
			out << stream.readLine();
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::SaveFile(const QStringList &data, const QString &fileName)
	{
		QFile file(FileName(fileName));
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))  ThrowRuntimeException("File not opened");
		if (data.size() == 0) return;
		QTextStream stream(&file);
		int lastItem = data.size() - 1;
		for(int i = 0; i < lastItem; ++i)
		{
			stream << data[i] << '\n';
		}
		stream << data[lastItem];
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::ParseCountryLine(const QStringList &line)
	{
		if (line.size() != 2) return;
		
		if (line[0].isEmpty() || line[1].isEmpty()) return;
		
		QString code(line[0].simplified());
		QString countryName(line[1].simplified());

		if (countryName.isEmpty() && code.isEmpty())	return;		

		if (code.contains(m_regexpNotDigit)) return;

		AddCountry(countryName, code);
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::AddCountry(const QString &countryName, const QString &code)
	{
		boost::shared_ptr<CountryFilter> country = SearchCountryFilter(countryName);
		if (country != 0) 
		{
			country->AddCode(code);
			return;
		}

		country.reset(new CountryFilter(*this, 
			Log().LogCreator(),
			countryName, 
			code,
			0));

		m_countryFilters.insert(countryName, country);
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::ParseCustomerLine(const QStringList &line)
	{
		if (line.size() != 2) return;

		if (line[0].isEmpty() || line[1].isEmpty())	return;
	
		QString customerName(line[0].simplified());
		QString number(line[1].simplified());

		if (!customerName.isEmpty() && !number.isEmpty())	AddCustomer(customerName, number);		
	}

	// -------------------------------------------------------------------------------

	boost::shared_ptr<NObjCallMeeting::CustomersSubSet>
		NObjCallMeeting::CustomersList(const QStringList &line) 
	{
		boost::shared_ptr<CustomersSubSet> customers(new CustomersSubSet);

		if (line.isEmpty()) ThrowRuntimeException("Customers line not found.");

		if (line[0] != " ") ThrowRuntimeException("First cell isn't empty.");

		for(int i = 1; i < line.size(); ++i)
		{
			boost::shared_ptr<CallMeetingCustomer> customer =
				SearchCustomer(line[i]);
			if (customer == 0) 
			{
				ThrowRuntimeException(QString("Customer '%1' not found").arg(line[i]));
			}
			customers->push_back(customer);
		}
		return customers;
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::CheckRuleLine(const QStringList &line,
		const CustomersSubSet &customers) 
	{
		if (line.empty()) return;

		if (SearchCountryFilter(line[0].simplified()) == 0)			
		{
			AsyncComplete(false, QString("Country '%1' not found").arg(line[0]));
			return;
		}

		if (line.size() - 1 != customers.size())
		{
			AsyncComplete(false, QString("Wrong rules for country '%1'").arg(line[0]));
			return;
		}

		for(int i = 0; i < customers.size(); ++i)
		{
			CallMeetingRulePar rule;
			if (!rule.LoadRule(line[i + 1], CRuleSplitter)) 
			{
				AsyncComplete(false, QString("Wrong rule for country '%1' and customer '%2'").
					arg(line[0]).arg(customers[i]->Name()));
				return;
			}
		}
	}

	// -------------------------------------------------------------------------------

	void NObjCallMeeting::ProcessRuleLine(const QStringList &line, 
		CustomersSubSet &customers)
	{
		if (line.empty()) return;

		ESS_ASSERT(customers.size() == line.size() -1);

		for(int i = 0; i < customers.size(); ++i)
		{
			CallMeetingRulePar rule;
			rule.LoadRule(line[i + 1], CRuleSplitter);
			customers[i]->SetRuleParameters(line[0].simplified(), rule);
		}
	}

};




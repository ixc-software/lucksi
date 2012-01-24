#include "stdafx.h" 
#include "CallMeetingState.h"

namespace
{
	using CallMeeting::CallMeetingState;

	static const int CVersion = 1;

	static QString CApplication()	{ return "CallMeeting"; }

	struct Header
	{
		static void Write(QSettings &settings)
		{
			settings.beginGroup("main");
			settings.setValue("Version", CVersion);
			settings.setValue("Application", CApplication());
			settings.setValue("Data", QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
			settings.endGroup();
		}

		static bool Read(QSettings &settings)
		{
			settings.beginGroup("main");

			bool result = false;

			int version = settings.value("Version", -1).toInt(&result);
			if(result) result = (version == CVersion);

			if(result)
			{
				QString app = settings.value("Application", "").toString();

				result = (app == CApplication());
			}

			settings.endGroup();

			return result;
		}
	};

	// ---------------------------------------------------------------------

	struct RuleHandler
	{

		typedef CallMeetingState::RuleList RuleList;

		typedef CallMeetingState::Rule Rule;

		static bool Read(QSettings &settings, RuleList &rules)
		{
			int size = settings.beginReadArray("Rules");

			bool result = false;

			for(int i = 0; i < size; ++i)
			{
				settings.setArrayIndex(i);

				Rule rule;

				rule.Country = settings.value("Country", "").toString();

				rule.AllowedCall = settings.value("AllowedCall").toInt(&result);
				if(!result) break;

				rule.CurrentIncomingCalls = settings.value("CurrentIncomingCalls").toInt(&result);
				if(!result) break;

				rule.CurrentAcceptedCalls = settings.value("CurrentAcceptedCalls").toInt(&result);
				if(!result) break;

				if(!rule.IsEmpty()) rules.push_back(rule);
			}

			settings.endArray();

			return result;
		}

		// ---------------------------------------------------------------------

		static void Write(QSettings &settings, const RuleList &rules)
		{
			ESS_ASSERT(rules.size() != 0);

			settings.beginWriteArray("Rules", rules.size());

			for(int i = 0; i < rules.size(); ++i)
			{
				ESS_ASSERT(!rules[i].IsEmpty());

				settings.setArrayIndex(i);
				settings.setValue("Country", rules[i].Country);
				settings.setValue("AllowedCall", rules[i].AllowedCall);
				settings.setValue("CurrentIncomingCalls", rules[i].CurrentIncomingCalls);
				settings.setValue("CurrentAcceptedCalls", rules[i].CurrentAcceptedCalls);		
			}

			settings.endArray();
		}
	};

	// ---------------------------------------------------------------------

	struct CustomerHandler
	{
		typedef CallMeetingState::CustomerList CustomerList;

		typedef CallMeetingState::Customer Customer;

		static bool Read(QSettings &settings, CustomerList &customers)
		{
			int size = settings.beginReadArray("Customers");

			bool result = false;

			for(int i = 0; i < size; ++i)
			{
				settings.setArrayIndex(i);

				Customer customer;

				customer.Name = settings.value("Name", "").toString();

				result = RuleHandler::Read(settings, customer.Rules);

				if(!result) break;

				if (!customer.Rules.isEmpty()) customers.push_back(customer);
			}

			settings.endArray();

			return result;
		}

		// ---------------------------------------------------------------------

		static void Write(QSettings &settings, const CustomerList &customers)
		{
			settings.beginWriteArray("Customers", customers.size());

			for(int i = 0; i < customers.size(); ++i)
			{
				settings.setArrayIndex(i);

				settings.setValue("Name", customers[i].Name);
				
				RuleHandler::Write(settings, customers[i].Rules);
			}

			settings.endArray();
		}

	};
}


namespace CallMeeting
{

	void CallMeetingState::Write(const CustomerList &customers)
	{
		{
			QFile file(StoreFilename());
			if(file.exists()) file.resize(0);
		}

		QSettings settings(StoreFilename(),  QSettings::IniFormat);

		Header::Write(settings);

		CustomerHandler::Write(settings, customers);
	}

	// ---------------------------------------------------------------------

	bool CallMeetingState::Read(CustomerList &customers)
	{
		QSettings settings(StoreFilename(),  QSettings::IniFormat);

		if(!Header::Read(settings)) return false;

		return CustomerHandler::Read(settings, customers);
	}

	// ---------------------------------------------------------------------

	QString CallMeetingState::StoreFilename()
	{
		static QString filename(QString("CmStore_V%1.tmp").arg(CVersion));
		
		return filename;
	}

}
#pragma once

#include "Utils/FullEnumResolver.h"
#include "Utils/ErrorsSubsystem.h"

namespace iSip
{
	
	class SipRegistrationState : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:

		enum Value
		{
			Disabled,
			RsNoRegistration,
			RsProgress,
			RsSuccess,
			RsRemoved
		};

		Q_ENUMS(Value);

		static bool IsReadyForCall(Value state) 
		{
			return state == SipRegistrationState::RsNoRegistration ||
				state == SipRegistrationState::RsSuccess;
		}

		static QString ToString(Value val) 
		{
			bool error = false;

			QString state = Utils::EnumResolve(val, &error).c_str();

			ESS_ASSERT(!error);

			return state;
		}
	};

}


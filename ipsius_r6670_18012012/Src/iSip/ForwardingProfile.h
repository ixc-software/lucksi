#pragma once

#include "resip/stack/Uri.hxx"

namespace iSip
{

	class ForwardingType  : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:

		enum Val 
		{
			None,
			Unconditional,
			OnBusy,
			OnFailure,
			OnNoAnswer
		};

		Q_ENUMS(Val);
	};

	class ForwardingProfile
	{
	public:
		
		ForwardingProfile() : 
			Type(ForwardingType::None),
			NoAnswerTimeout(0)
		{}

		ForwardingType::Val Type;

		resip::Uri Destination;

		int NoAnswerTimeout;

	};

}
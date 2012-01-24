#pragma once 

#include "stdafx.h"
#include "Utils/QtEnumResolver.h"

namespace Dss1ToSip
{

	class TypeOfNumber : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:
		enum Value
		{
			Unknown,
			International,
			National,
			NetSpecific,
			Subscriber,
			Abbreviated,
		};

		Q_ENUMS(Value);

	};

	class NumberingPlan : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:
		enum Value
		{
			Unknown,
			ISDN,
			Data,
			Telex,
			National,
			Private
		};
		
		Q_ENUMS(Value);
	
	};

	class PresentInd : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:
		enum Value
		{
			Allowed, //
			Restricted,
			NotAvailable,
			PesenIndReserved
		};
		
		Q_ENUMS(Value);
	};

	class ScreeningInd : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:
		enum Value
		{
			UserProv_NotScreened, //
			UserProv_Passed,
			UserProv_Failed,
			NetProv
		};

		Q_ENUMS(Value);
	};

	class HardType : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:
		enum Value
		{
			ErrorHardType,
			T1,
			E1
		};

		Q_ENUMS(Value);
	};

	class Dss1Def : boost::noncopyable
	{
	public:

		static const int CDChannelNPos = -1;
		static const int CDChannelDef = 16;

		template<class T> static typename T::Value 
			Resolve(const char *str, bool &ret)
		{
			return 
				Utils::QtEnumResolver<T, typename T::Value>().Resolve(str, &ret);
		}

		template <class T> static std::string ToString(typename T::Value value)
		{
            return Utils::QtEnumResolver<T, typename T::Value>().Resolve(value);
		}

	};
};



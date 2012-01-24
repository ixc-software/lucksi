#pragma once

#include "Utils/ErrorsSubsystem.h"

#include "rutil/Data.hxx"

namespace resip 
{	
	class SipMessage;
	class Uri;
}

namespace iSip
{
	class SipUtils  : public QObject,
		boost::noncopyable
	{
		Q_OBJECT;

	public:
	
		enum PrintMode 
		{
			Short,
			Detail,
			Resip
		};

		Q_ENUMS(PrintMode);

		static resip::Data ToResipData(const std::string &str)
		{
			return resip::Data(str.c_str());
		}

		// -------------------------------------------------------------------------------------

		static resip::Data ToUtf8(const QString &str)
		{
			return resip::Data(str.toUtf8().constData());
		}
	
		// -------------------------------------------------------------------------------------

		template <class T> static QString ToQString(const T &data)
		{
			// todo correct 
			return ToString(data).c_str();
		}

		// -------------------------------------------------------------------------------------
		
		template <class T> static std::string ToString(const T &data)
		{
			// todo correct 
			std::ostringstream out;
			out << data;
			return out.str();
		}

		// -------------------------------------------------------------------------------------

		static std::string ToString(PrintMode mode, const resip::SipMessage &msg);

		// -------------------------------------------------------------------------------------
		
		static bool ConvertStrToUri(const QString &val, resip::Uri &uri, std::string &error);

		// -------------------------------------------------------------------------------------

        static std::string ResponseCodeToString(int responseCode);
	};

}


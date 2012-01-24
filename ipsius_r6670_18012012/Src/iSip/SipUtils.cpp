#include "stdafx.h"
#include "iSip/undeferrno.h"
#include "SipUtils.h"
#include "resip/stack/SipMessage.hxx"

namespace iSip
{

	std::string SipUtils::ToString( PrintMode mode, const resip::SipMessage &msg )
	{
		if (mode == Resip) return ToString(msg);

		resip::NameAddr to = msg.header(resip::h_To);
		resip::NameAddr from = msg.header(resip::h_From);

		if (mode == Detail)
		{
			std::string str;

			if (msg.isRequest()) 
			{
				str += std::string("Request line: ") + 
					ToString(msg.header(resip::h_RequestLine)) +
					"; ";
			}

			str += "To: " + ToString(to) + 
				"; From: " + ToString(from);

			if (msg.exists(resip::h_Contacts)) 
			{
				str += "; Contact: " + ToString(msg.header(resip::h_Contacts).front());
			}

			return str;
		}

		if (mode == Short)
		{
			return "To: " + ToString(to.uri().user()) +
				" From: " + ToString(from.uri().user());
		}

		ESS_HALT("");
		return "";
	}

	// -------------------------------------------------------------------------------------

	bool SipUtils::ConvertStrToUri( const QString &val, resip::Uri &uri, std::string &error )
	{
		try
		{
			uri = resip::Uri(ToUtf8(val));
		}
		catch(resip::ParseException &e)
		{
			error = std::string(e.name()) + " Context: " + e.getContext().c_str();
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------

	std::string SipUtils::ResponseCodeToString(int responseCode)
	{
		switch(responseCode)
		{
			// 1xxInformational Responses
			case 100: return "Trying";
			case 180: return "Ringing";
			case 181: return "Call Is Being Forwarded";
			case 182: return "Queued";
			case 183: return "Session Progress";

				// 2xxSuccessful Responses
			case 200: return "OK";
			case 202: return "accepted: It Indicates that the request has been understood but actually can't be processed";
			case 204: return "No Notification [RFCcase 5839]";

				// 3xxRedirection Responses
			case 300: return "Multiple Choices";
			case 301: return "Moved Permanently";
			case 302: return "Moved Temporarily";
			case 305: return "Use Proxy";
			case 380: return "Alternative Service";

				// 4xxClient Failure Responses
			case 400: return "Bad Request";
			case 401: return "Unauthorized (Used only by registrars or user agents. Proxies should use proxy authorization case 407)";
			case 402: return "Payment Required (Reserved for future use)";
			case 403: return "Forbidden";
			case 404: return "Not Found (User not found)";
			case 405: return "Method Not Allowed";
			case 406: return "Not Acceptable";
			case 407: return "Proxy Authentication Required";
			case 408: return "Request Timeout (Couldn't find the user in time)";
			case 409: return "Conflict";
			case 410: return "Gone (The user existed once, but is not available here any more.)";
			case 412: return "Conditional Request Failed";
			case 413: return "Request Entity Too Large";
			case 414: return "Request-URI Too Long";
			case 415: return "Unsupported Media Type";
			case 416: return "Unsupported URI Scheme";
			case 417: return "Unknown Resource-Priority";
			case 420: return "Bad Extension (Bad SIP Protocol Extension used, not understood by the server)";
			case 421: return "Extension Required";
			case 422: return "Session Interval Too Small";
			case 423: return "Interval Too Brief";
			case 424: return "Bad Location Information";
			case 428: return "Use Identity Header";
			case 429: return "Provide Referrer Identity";
			case 433: return "Anonymity Disallowed";
			case 436: return "Bad Identity-Info";
			case 437: return "Unsupported Certificate";
			case 438: return "Invalid Identity Header";
			case 480: return "Temporarily Unavailable";
			case 481: return "Call/Transaction Does Not Exist";
			case 482: return "Loop Detected";
			case 483: return "Too Many Hops";
			case 484: return "Address Incomplete";
			case 485: return "Ambiguous";
			case 486: return "Busy Here";
			case 487: return "Request Terminated";
			case 488: return "Not Acceptable Here";
			case 489: return "Bad Event";
			case 491: return "Request Pending";
			case 493: return "Undecipherable (Could not decrypt S/MIME body part)";
			case 494: return "Security Agreement Required";

				// 5xxServer Failure Responses
			case 500: return "Server Internal Error";
			case 501: return "Not Implemented: The SIP request method is not implemented here";
			case 502: return "Bad Gateway";
			case 503: return "Service Unavailable";
			case 504: return "Server Time-out";
			case 505: return "Version Not Supported: The server does not support this version of the SIP protocol";
			case 513: return "Message Too Large";
			case 580: return "Precondition Failure";

				// 6xxGlobal Failure Responses
			case 600: return "Busy Everywhere";
			case 603: return "Decline";
			case 604: return "Does Not Exist Anywhere";
			case 606: return "Not Acceptable";
		}

		return "Unknown code";
	}
}


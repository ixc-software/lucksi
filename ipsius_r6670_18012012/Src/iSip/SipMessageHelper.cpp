#include "stdafx.h"
#include "iSip/undeferrno.h"

#include "SipMessageHelper.h"
#include "SipUtils.h"

#include "resip/stack/Helper.hxx"
#include "resip/stack/SdpContents.hxx"
#include "resip/stack/SipMessage.hxx"

#include "Utils/ErrorsSubsystem.h"

namespace {	resip::Uri DefaultUri;	};

namespace iSip
{

    SipMessageHelper::SipMessageHelper(const resip::SipMessage &msg) : m_msg(msg)
    {}

	// ----------------------------------------------------------------------------

	SipMessageHelper::Uri SipMessageHelper::RequestUri() const
	{
		ESS_ASSERT(m_msg.isRequest());

		return Uri(m_msg.header(resip::h_RequestLine).uri());
	}

	// ----------------------------------------------------------------------------


    SipMessageHelper::NameAddr SipMessageHelper::To() const
    {
        ESS_ASSERT(m_msg.exists(resip::h_To));

        return NameAddr(m_msg.header(resip::h_To));
    }

    // ----------------------------------------------------------------------------

    SipMessageHelper::NameAddr SipMessageHelper::From() const
    {
        ESS_ASSERT(m_msg.exists(resip::h_From));

        return NameAddr(m_msg.header(resip::h_From));
    }

    // ----------------------------------------------------------------------------

    SipMessageHelper::NameAddr SipMessageHelper::Contact() const
    {
        ESS_ASSERT(m_msg.exists(resip::h_Contacts));

        return NameAddr(m_msg.header(resip::h_Contacts).front());
    }

	// ----------------------------------------------------------------------------

	std::auto_ptr<resip::SdpContents> SipMessageHelper::Sdp() const
	{
		resip::SipMessage msg(m_msg);
		return resip::Helper::getSdp(msg.getContents());
	}

    // ----------------------------------------------------------------------------

    SipMessageHelper::Uri::Uri(const resip::Uri &uri) : m_uri(uri)
    {}

    // ----------------------------------------------------------------------------

    std::string SipMessageHelper::Uri::host() const
    {
		return SipUtils::ToString(m_uri.host());
    }

    // ----------------------------------------------------------------------------

    std::string SipMessageHelper::Uri::user() const
    {
		return SipUtils::ToString(m_uri.user());
    }

    // ----------------------------------------------------------------------------

    std::string SipMessageHelper::Uri::userParameters() const
    {
		return SipUtils::ToString(m_uri.userParameters());
    }

    // ----------------------------------------------------------------------------

    std::string SipMessageHelper::Uri::scheme() const
    {
		return SipUtils::ToString(m_uri.scheme());
    }

    // ----------------------------------------------------------------------------

    int SipMessageHelper::Uri::port() const
    {
        return m_uri.port();
    }

	// ----------------------------------------------------------------------------

	std::string SipMessageHelper::Uri::ToString() const
	{
		return SipUtils::ToString(m_uri);
	}

	// ----------------------------------------------------------------------------

	SipMessageHelper::NameAddr::NameAddr(const resip::NameAddr &nameAddress) :
		m_nameAddr(nameAddress),
		m_uri(nameAddress.uri())
	{}

	// ----------------------------------------------------------------------------

	std::string SipMessageHelper::NameAddr::displayName() const
	{
		return SipUtils::ToString(m_nameAddr.displayName());
	}

	// ----------------------------------------------------------------------------

	std::string SipMessageHelper::NameAddr::ToString() const
	{
		return SipUtils::ToString(m_nameAddr);
	}

};


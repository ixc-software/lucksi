#pragma once

#include "stdafx.h"
#include "SipUtils.h"

namespace resip
{
    class SipMessage;
    class Uri;
	class NameAddr;
	class SdpContents;
};

namespace iSip
{
	class SipMessageHelper : boost::noncopyable
    {
    public:

        class Uri 
        {
        public:
            
			Uri(const resip::Uri &uri);
            std::string host() const;
            std::string user() const;
            std::string userParameters() const;
            std::string scheme() const;
            int port() const;

			std::string ToString() const;

        private:
            const resip::Uri &m_uri;
        }; 

        class NameAddr
		{
		public:
			NameAddr(const resip::NameAddr &nameAddr);

            std::string displayName() const;

			const Uri &uri() const
			{
				return m_uri;
			}

			std::string ToString() const;

		private:
			const resip::NameAddr &m_nameAddr;
			Uri m_uri;
		};
	
	public:

        SipMessageHelper(const resip::SipMessage &msg);



	public:
		Uri RequestUri() const;
        NameAddr To() const;
        NameAddr From() const;
        NameAddr Contact() const;

        std::auto_ptr<resip::SdpContents> Sdp() const;

	private:
        const resip::SipMessage &m_msg;
    };

};


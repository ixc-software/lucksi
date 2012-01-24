#include "stdafx.h"
#include "ObjectLinkTestDetail.h"

namespace
{
    typedef ObjLink::IDialInterface TIntf;

    class LocalIntfProxy : 
        public iCore::MsgObject, 
        public TIntf
    {
        typedef LocalIntfProxy T;

        ObjLink::ILocalProxyHost &m_host;
        TIntf *m_pI;

    // TIntf impl
    private:

        void Dial(boost::shared_ptr<QString> number) 
        { 
            if (m_host.ConnectedForProxy())
                m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::Dial, number) );
        }

        void SetRegionCode(int code) 
        { 
            if (m_host.ConnectedForProxy())
                m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::SetRegionCode, code) );
        }

        void EndCall() 
        { 
            if (m_host.ConnectedForProxy())
                m_host.SendMsgToDestinationObject( CreateMsg(m_pI, &TIntf::EndCall) );
        }

    public:

        LocalIntfProxy(ObjLink::ILocalProxyHost &host)
            : iCore::MsgObject(host.getMsgThreadForProxy()),
            m_host(host)
        {
            m_pI = ObjLink::LocalProxyHostToInterface<TIntf>(host);
        }

    };

    // -------------------------------------------------------

    // ObjLink::RegisterObjectLinkMeta<TIntf, LocalIntfProxy> RM(0);
	
}  // namespace


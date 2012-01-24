#include "stdafx.h"
#include "BChannelsWrapper.h"


namespace ISDN
{

    BChannelsWrapper::BChannelsWrapper( IIsdnIntf& channOwner, const SetBCannels& chan )
        : m_ChannOwner(channOwner),
        m_chan(chan)
    {
    }

    // ------------------------------------------------------------------------------------

    BChannelsWrapper::~BChannelsWrapper()
    {
        m_ChannOwner.FreeBChannel(m_chan);
    }

    // ------------------------------------------------------------------------------------

    const SetBCannels& BChannelsWrapper::getChans() const
    {
        return m_chan;
    }

    // ------------------------------------------------------------------------------------

    int BChannelsWrapper::getIntfId() const
    {
        return m_ChannOwner.NumberInterface();
    }

    // ------------------------------------------------------------------------------------

    IeConstants::UserInfo BChannelsWrapper::getPayload() const
    {
        return m_ChannOwner.getPayload();
    }

} // namespace ISDN


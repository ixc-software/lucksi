#ifndef BCHANNELSWRAPPER_H
#define BCHANNELSWRAPPER_H

#include "SetBCannels.h"
#include "iisdnintf.h"

namespace ISDN
{

    // ������ 2 ������: 1 ���������� ���� chan+intf, 2 - RAII ������� ������������ ���������� ������
    class BChannelsWrapper : boost::noncopyable // -- use BaseSpecAlloc ?? -- �� ��������� �� ������ ������� � ��� �������
    {           
        IIsdnIntf& m_ChannOwner;        
        SetBCannels m_chan;                

    public:

        BChannelsWrapper(IIsdnIntf& channOwner, const SetBCannels& chan);

        ~BChannelsWrapper();

        const SetBCannels& getChans() const;

        int getIntfId() const;

        IeConstants::UserInfo getPayload() const;;
    };
    
    typedef boost::shared_ptr<BChannelsWrapper> SharedBChansWrapper;

} // namespace ISDN

#endif




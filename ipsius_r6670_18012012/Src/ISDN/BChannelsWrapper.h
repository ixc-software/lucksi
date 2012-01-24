#ifndef BCHANNELSWRAPPER_H
#define BCHANNELSWRAPPER_H

#include "SetBCannels.h"
#include "iisdnintf.h"

namespace ISDN
{

    // решает 2 задачи: 1 компоновка пары chan+intf, 2 - RAII обертка возвращающая выделенные каналы
    class BChannelsWrapper : boost::noncopyable // -- use BaseSpecAlloc ?? -- не использую тк утечка каналов и так заметна
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




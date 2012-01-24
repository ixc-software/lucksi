#ifndef ISDNINTF_H
#define ISDNINTF_H

#include "SetBCannels.h"
#include "iisdnintf.h"

namespace ISDN
{
	class IsdnIntf :public IIsdnIntf
    {
        SetBCannels m_free;
        SetBCannels m_busy;
		SetBCannels m_blocking;
        int m_number;

    public:

        IsdnIntf(const SetBCannels& chMap, int number) : m_free(chMap),
            m_number(number)
		{}
        void Blocking(const SetBCannels &set);        
        void Unblocking(const SetBCannels &set);
        const SetBCannels& getFree() const;
        SetBCannels& getFree();
		ISDN::IeConstants::UserInfo getPayload() const;
        
	// IIsdnIntf
		int NumberInterface() const;
		SetBCannels CaptureBChannel(int count);
		bool CaptureBChannel(const SetBCannels &set);

        void FreeBChannel(const SetBCannels &set);

    };       

} // namespace ISDN

#endif



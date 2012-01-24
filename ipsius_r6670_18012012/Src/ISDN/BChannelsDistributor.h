#ifndef BChannelsDistributor_H
#define BChannelsDistributor_H

#include "stdafx.h"
#include "Utils/SafeRef.h"

namespace ISDN
{
    using boost::shared_ptr;

    class DssCallParams;
    class IeChannelIdentification;
    class L3Packet;
    class L3Call;
    class IBDistribStrategy;
    class IIsdnL3Internal;

    typedef shared_ptr<IeChannelIdentification> SharedIeChan;
    class IDssToGroup;


    // –аспределитель ресурсов(форммирователь запросов / обработчик ответов в процедурах назначени€ каналов)
    class BChannelsDistributor : boost::noncopyable
    {            
        Utils::SafeRef<IDssToGroup> m_intfGroup;
        IIsdnL3Internal& m_IL3;

    public:
        // создает Ie дл€ запроса нужного числа каналов в соотв тек стратегии,
        // может зарезервировать запрошенные каналы. ¬случае неудачи возвращает пустой shared_ptr.
        SharedIeChan CreateInquiry(shared_ptr<const DssCallParams> pInParams, L3Call* pCall);

        // принимает Ie-запроса и, если возможно, занимает запрошенные каналы, формиру€ Ie-ответа.
        // ¬случае неудачи возвращает пустой shared_ptr.
        SharedIeChan CreateResponse(const L3Packet& incoming, L3Call* pCall);

        // ќбрабатывает Ie-ответа, фиксиру€ назначенные каналы
        bool ProcessResponse(const L3Packet& incoming, L3Call* pCall);        

        BChannelsDistributor(Utils::SafeRef<IDssToGroup> intfGroupe, IIsdnL3Internal& IL3)
            : m_intfGroup(intfGroupe),
            m_IL3(IL3)
        {}
    };
} // namespace ISDN

#endif


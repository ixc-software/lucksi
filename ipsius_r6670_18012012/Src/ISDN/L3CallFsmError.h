#ifndef L3CALLFSMERROR_H
#define L3CALLFSMERROR_H

#include "Utils/IBasicInterface.h"
#include "IeConstants.h"

namespace ISDN
{
    class L3CallFsm;
        
    // базовый класс ошибок обработки типизированного пакета
    class PacketExceptProcessor : Utils::IBasicInterface
    {
    public:
        virtual void DoAction(L3CallFsm& call) = 0;
    };

    //class L3CallFsm::SendStatus : public PacketExceptProcessor
    //{        
    //    IeConstants::CauseNum m_num;

    //    void DoAction(L3CallFsm& call) // override
    //    {
    //        PacketStatus pack(call.m_IL3, call.m_pOwner->GetCallRef());
    //        IeCause ieCause( DssCause::Create(&call.m_IL3, m_num) );
    //        pack.AddIe(ieCause);
    //        call.Send(pack);
    //    }

    //public:
    //    SendStatus(IeConstants::CauseNum num) : m_num(num)
    //    {}
    //};

    class ErrActionCloseCall : public PacketExceptProcessor
    {              
        IeConstants::CauseNum m_num;
        void DoAction(L3CallFsm& call); // override;

    public:
        ErrActionCloseCall(IeConstants::CauseNum num) : m_num(num)
        {}
    };

    // исключение выбрасываемое обработчиками событий пакетов
    ESS_TYPEDEF_T(ErrPacketProcess, boost::shared_ptr<PacketExceptProcessor>);


} // namespace ISDN

#endif

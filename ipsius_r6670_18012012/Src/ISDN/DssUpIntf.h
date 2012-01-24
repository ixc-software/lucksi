#ifndef DSSUPINTF_H
#define DSSUPINTF_H

#include "stdafx.h"

#include "ObjLink/ObjectLinkBinder.h"
#include "ObjLink/ObjLinkInterfaces.h"

#include "DssWarning.h"
#include "DssCause.h"
#include "SetBCannels.h"

// TODO - какие инклуды можно убрать

namespace ISDN
{   
        
    using boost::shared_ptr;

    class IDssCallEvents;
    class IDssCall;   

    typedef boost::shared_ptr<const ObjLink::ObjLinkBinder<IDssCallEvents> > BinderToUserCall;
    typedef boost::shared_ptr<const ObjLink::ObjLinkBinder<IDssCall> > BinderToDssCall;

    //========================================================================================================
    //                                      Упаковки параметров:
    //========================================================================================================

    // определения упаковок параметров для интерфейса
    class IDssCallEvents_ParamsShells
    {
        friend class IDssCallEvents;
        // упаковка параметров длы DssCallCreated()
        struct DssCallCreatedParametrs        
        {            
            const BinderToDssCall m_linkBinder;
            const SetBCannels m_setBCannels;            
            const int m_bChansIntfId;
            
            DssCallCreatedParametrs(BinderToDssCall linkBinder, const BChannelsWrapper& channels)
                : m_linkBinder(linkBinder),
                m_setBCannels(channels.getChans()),
                m_bChansIntfId(channels.getIntfId())
            {}            
        };

        struct Info
        {
            DssPhoneNumber Num;
            Utils::InitProp<IeConstants::SignalValue> Signal;
            bool SendingComplete; // досылка номера закончена
        };
    };

    // Упаковки параметров для вызовоа методов интерфейса ICallbackDss
    class ICallBackDss_ParamsShells
    {    
        friend class ICallbackDss;

        struct IncommingCallParametrs
        {
            const BinderToDssCall m_bindToDssCall;
            const shared_ptr<const DssCallParams> m_callParams;
            const std::string m_unicCallName; // for trace

            IncommingCallParametrs(BinderToDssCall bindToDssCall, shared_ptr<const DssCallParams> callParams, const std::string unicCallName)
                : m_bindToDssCall(bindToDssCall),
                m_callParams(callParams),
                m_unicCallName(unicCallName)
            {}
        };

        struct CompleteInfo 
        {
            bool m_ok;
            std::string m_addInfo;
            CompleteInfo(bool Ok, const std::string& addInfo = "") : m_ok(Ok), m_addInfo(addInfo)
            {}
        };
    };
   

    // Упаковки параметров для вызовоа методов интерфейса ILayerDss
    class ILayerDss_ParamsShells
    {    
        friend class ILayerDss;

        struct CreateOutCallParametrs
        {
            const BinderToUserCall m_bindToDssCall;
            const shared_ptr<const DssCallParams> m_callParams;
            const std::string m_unicCallName;

            CreateOutCallParametrs(BinderToUserCall bindToDssCall, shared_ptr<const DssCallParams> callParams, const std::string& unicCallName = "")
                : m_bindToDssCall(bindToDssCall),
                m_callParams(callParams),
                m_unicCallName(unicCallName)
            {}
        };
    };

    //========================================================================================================
    //                                      Определения интерфейсов:
    //========================================================================================================
    class ForceMoc_DssUpIntf : QObject { Q_OBJECT };
    

    // интерфейс вызова, пользователь (IDssCallEvents) -> вызов (IDssCall)
    class IDssCall : public ObjLink::IObjectLinkInterface
    {

    public:

        virtual void HoldReq() = 0;
        virtual void HoldConf() = 0;
        virtual void RetriveReq() = 0;
        virtual void RetriveConf() = 0;

        // Принята вся адресная информация. Выполнить ProceedingReq.
        virtual void AddressComplete() = 0;

        // Принята не вся адресная информация. Запрос дополнительной адрессной информации через IDssCallEvents::MoreInfo().
        virtual void AddressIncomplete() = 0;

        // связывание с входящим вызовом
        virtual void SetLink(BinderToUserCall linkBinderToUserCall) = 0;        

        // Порвать связь. DssCall будет удален.
        virtual void Disconnent( boost::shared_ptr<const DssCause> pCause ) = 0;       

        // абонент найден, ему послали вызов  ???
        virtual void Alerting() = 0;

        // Пользователь принял вызов
        virtual void ConnectRsp() = 0;

    };

    // ------------------------------------------------------------


    // события вызова, вызов (IDssCall) -> пользователь (IDssCallEvents)
    class IDssCallEvents : public ObjLink::IObjectLinkInterface
    {        

    public:

        typedef IDssCallEvents_ParamsShells::DssCallCreatedParametrs DssCallCreatedParametrs;        
        typedef IDssCallEvents_ParamsShells::Info Info;

        virtual void HoldInd() = 0;  // вызов поставлен на удержание
        virtual void RetriveInd() = 0; // вызов снят с удержания      

        virtual void Alerting() = 0; // == КПВ 
        virtual void ConnectConfirm() = 0;  // соединение установленно ???(вызывающая сторона таки подключена)
        virtual void Connected() = 0;  // разговор

        // вызов создан, ответ станции, назначенные каналы
        virtual void DssCallCreated(DssCallCreatedParametrs params) = 0;

        virtual void DssCallErrorMsg(boost::shared_ptr<const CallWarning> pWarning) = 0;

        // принята дополнительная информация об устанавливаемом соединении
        virtual void MoreInfo( Info info ) = 0;

        virtual void ProgressInd(IeConstants::ProgressDscr descr) = 0;

        // аналогично IDssCall
        virtual void Disconnent(boost::shared_ptr<const DssCause> pCause) = 0;
    };

    // ------------------------------------------------------------

    // DSS (ILayerDss) -> пользователь (ICallbackDss)
    class ICallbackDss : public ObjLink::IObjectLinkInterface
    {
    public:

        typedef ICallBackDss_ParamsShells::IncommingCallParametrs IncommingCallParametrs;
        typedef ICallBackDss_ParamsShells::CompleteInfo CompleteInfo;

        // L2 Connected/Disconnected
        virtual void Activated() = 0;
        virtual void Deactivated() = 0;        

        virtual void IncomingCall(IncommingCallParametrs params) = 0; // Ringing
        virtual void DssErrorMsg(shared_ptr<const DssWarning> pWarning) = 0;
        virtual void RestartComplete(CompleteInfo info) = 0;

    };

    // ------------------------------------------------------------

    // интерфейс пользователь (ICallbackDss) -> DSS (ILayerDss)
    class ILayerDss : public ObjLink::IObjectLinkInterface
    {
    public:

        typedef ILayerDss_ParamsShells::CreateOutCallParametrs CreateOutCallParametrs;
        
        // ConnectToL2/DisconectL2
        virtual void Activate() = 0;
        virtual void Deactivate() = 0;

        // новый исходящий вызов (если вызов может быть совершен, DSS посылает SETUP)
        virtual void CreateOutCall(CreateOutCallParametrs params) = 0;
        // Освобождение каналов (сброс вызовов), отправка Restart, ожидание RestartAck
        virtual void RestartReq() = 0;
    };


    // ------------------------------------------------------------
    
    typedef shared_ptr<ObjLink::ObjLinkBinder<ICallbackDss> > BinderToICallBackDss;
    typedef shared_ptr<ObjLink::ObjLinkBinder<ILayerDss> > BinderToILayerDss;
    

} // ISDN

#endif

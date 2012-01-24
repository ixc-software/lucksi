#include "stdafx.h"

#include "ISDN/DssCause.h"
#include "DssSubScenario.h"

namespace IsdnTest
{
    using boost::shared_ptr;    
    using iCore::MsgTimer;

    using ISDN::DssPhoneNumber;
    using ISDN::IDssCall;
    using ISDN::DssCallParams;
    using ISDN::IsdnStack;
    using ISDN::DssCause;
    using ISDN::BinderToDssCall;

    DssSubScenario::DssSubScenario(IDssScenarioBase& owner, const DssPhoneNumber& numReceiver, OutgoingSide side,
        Timeout timeout, bool enableRndDrop)
        : MsgObject( owner.GetIDomain().getDomain().getMsgThread() ) ,
        m_side(side),
        m_owner(owner),
        m_leftSt( owner.GetLeftTEstack() ),
        m_rightSt( owner.GetRightNTstack() ),
        m_numReceiver(numReceiver),
        m_timeout(timeout),
        m_enableRndDrop(enableRndDrop),
        m_tTimeout(this, &DssSubScenario::OnTimeout),
        m_tDrop(this, &DssSubScenario::OnDrop)        
    {}

    //---------------------------------------------------------------------------

    DssPhoneNumber& DssSubScenario::getNumReceiver()
    {
        return m_numReceiver;
    }

    //---------------------------------------------------------------------------

    const Logger& DssSubScenario::getLogger()
    {
        ESS_ASSERT(m_pLog.get());
        return *(m_pLog.get());
    }

    //---------------------------------------------------------------------------

    IsdnInfraTest& DssSubScenario::getInfra()
    {
        return m_owner.GetInfra();
    }

    //---------------------------------------------------------------------------

    void DssSubScenario::DeleteOutCall()
    {            
        m_pOutCall.reset();
        m_pLog->Log("OutCall deleted", m_logTags.General);
        //TODO если отдать кудато shared_ptr, то не сработает !!!!
    }

    //---------------------------------------------------------------------------

    void DssSubScenario::DeleteInCall()
    {        
        m_pInCall.reset();
        m_pLog->Log("InCall deleted", m_logTags.General);
        //TODO если отдать кудато shared_ptr, m_pInCall то не сработает !!!!
    }

    //---------------------------------------------------------------------------

    void DssSubScenario::AsyncDeleteOutCall()
    {      
        PutMsg(this, &DssSubScenario::DeleteOutCall);
    }

    //---------------------------------------------------------------------------

    void DssSubScenario::AsyncDeleteInCall()
    {      
        PutMsg(this, &DssSubScenario::DeleteInCall);
    }

    //---------------------------------------------------------------------------

    IDssScenarioBase& DssSubScenario::GetIScenario()
    {
        return m_owner;
    }

    //---------------------------------------------------------------------------

    void DssSubScenario::OnTimeout(MsgTimer* )
    {   
        if ( IsComplete() )
        {
            m_tDrop.Stop();
            return;
        }
        Writer(*m_pLog, m_logTags.Error).Write() << "Timeout!";
        TUT_ASSERT(0 && "Timeout In Sub scenario!");
    }

    //---------------------------------------------------------------------------

    void DssSubScenario::OnDrop(MsgTimer* )
    {
        if ( IsComplete() ) // если тест уже выполнен                   
            return;

        ESS_ASSERT(m_pInCall.get() != 0 && m_pOutCall.get() != 0);

        // random causenum:
        //int num = m_owner.GetRandom().Next(99) + 1;

        ISDN::IeConstants::CauseNum num = ISDN::IeConstants::TemporaryFailure;
        shared_ptr<const DssCause> cause (
            DssCause::Create(m_owner.GetInfra(), ISDN::IeConstants::RLN, ISDN::IeConstants::NETWORK_SPECIFIC, num) );

        // random select side to drop
        if (m_owner.GetRandom().Next(49) > 24)
        {
            Writer(*m_pLog, m_logTags.General).Write() << "**Drop!** on incoming side:\n";
            m_pInCall->Drop(cause, *m_pOutCall);                
        }
        else
        {
            Writer(*m_pLog, m_logTags.General).Write() << "**Drop!** on outgoing side:\n";                
            m_pOutCall->Drop(cause, *m_pInCall);                
        }
    }

    //---------------------------------------------------------------------------
    
    void DssSubScenario::Run()
    {              
        m_pLog.reset( 
            //new Logger(getSubsName() + " ReceiverNo: " + m_numReceiver.getDigits(),
                //m_owner.GetLogStoreIntf(), true, m_owner.GetLogSessionProf() )

            new Logger(m_owner.GetLogger(), getSubsName() + " ReceiverNo: " + m_numReceiver.Digits().getAsString())
        );

        m_logTags.Init(*m_pLog);
        
        RegCalls();

        //Run
        ESS_ASSERT(m_pOutCall.get() != 0);
        m_pOutCall->Run();
        if(m_pInCall.get())
            m_pInCall->Run();

        if (CNoTimeout != m_timeout)
            m_tTimeout.Start(m_timeout);

        if (m_enableRndDrop) // Drop in range (10, timeout/10):
            m_tDrop.Start( m_owner.GetRandom().Next(m_timeout/5)+10 );
        
    }

    //---------------------------------------------------------------------------

    bool DssSubScenario::IsComplete()
    {
        bool inComplete = (m_pInCall.get() == 0);
        if (!inComplete)
        {
            inComplete = m_pInCall->IsComplete();
        }

        bool outComplete = (m_pOutCall.get() == 0);            
        if (!outComplete)
        {
            outComplete = m_pOutCall->IsComplete();
        }

        return inComplete && outComplete;
    }

    //---------------------------------------------------------------------------

    void DssSubScenario::IncomingCall(BinderToDssCall dssCall, shared_ptr<const DssCallParams> pParams, IsdnStack* from)
    {        
        ESS_ASSERT(pParams->GetCalled().Digits() == m_numReceiver.Digits());
        TUT_ASSERT(( (m_side == rightSender) ? &m_leftSt : &m_rightSt ) == from);

        if ( !m_pInCall.get() )
            TUT_ASSERT ( m_pInCall.get() && "Unexpected Incoming Call" );
        m_pInCall->OnIncomingCall(dssCall, pParams);            
    }

    //---------------------------------------------------------------------------

    std::string DssSubScenario::getSubsName()
    {            
        return Platform::FormatTypeidName(typeid(*this).name());
        //TODO IsdnTest:: убрать
    }

} // namespace IsdnTest



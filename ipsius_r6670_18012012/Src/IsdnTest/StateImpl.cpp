
#include "stdafx.h" 
// #include "IsdnTestConfig.h"

#include "TestState.h"
#include "LapdTestClass.h"
#include "DataExchangeTestState.h"



#define DEF_CLASS(name, maxDuration) class name: public TestState		\
	{		\
	public:		\
        name(LapdTest &fsm):TestState(fsm, #name , (maxDuration) ){}		\
	protected:		\
		void Do(); /*override*/		\
	}		

namespace IsdnTest
{
    namespace TestStateImpl
    {
        enum
        {
            CLongTimeout = 5000,
            CShortTimeout = 100,
            CInifityTimeout = -1           
        };

        DEF_CLASS(Initial, CLongTimeout);
    
	    DEF_CLASS(WaitEstablishConf, CLongTimeout);

        //DEF_CLASS(ExchangeIPacket, CShortTimeout);

        //DEF_CLASS(WaitFeedData, CShortTimeout);

        DEF_CLASS(ReleaseReq, CInifityTimeout);

        DEF_CLASS(WaitDisconnect, CLongTimeout); 

        DEF_CLASS(EndState, CInifityTimeout);

    }; //namespace TestStateImpl
}; //namespace IsdnTest


#undef DEF_CLASS
//(name, maxDuration)


//----------------------------------------------------------------------

namespace IsdnTest
{
    namespace TestStateImpl
    {
        using ISDN::IL3ToL2;

	    void Initial::Do()  //переименовать в название сценария
	    {            
            //isx
            IL3ToL2 &L2 = GetOwner().GetL2Left();
            GetOwner().GetL3Left().SetStateWaitEstConf();
            L2.EstablishReq();
		    SwitchState( new WaitEstablishConf(GetOwner()) );

            /*IL3ToL2 &L2 = GetOwner().GetL2Right();
            GetOwner().GetL3Right().SetStateWaitEstConf();
            L2.EstablishReq();
            SwitchState( new WaitEstablishConf(GetOwner()) );*/
	    }

        //-------------------------------------------------------------

	    void WaitEstablishConf::Do()
	    {   
            LapdTest &Fsm = GetOwner();
    
            if ( !GetOwner().GetL3Left().Complete() ) return;

            TUT_ASSERT(Fsm.GetL2Left().getL2()->IsST_L2_7());
            TUT_ASSERT(Fsm.GetL2Right().getL2()->IsST_L2_7());
           
            //int cycle = GetOwner().GetProfile().DataExchangeCycle;
            SwitchState( GetDataExchangeTest(GetOwner(), -1, new ReleaseReq(GetOwner()) ) );
            //SwitchState (new ReleaseReq(GetOwner()));
        }

        //-------------------------------------------------------------

        /*void ExchangeIPacket::Do()
        {
            L2Shell &L2 = GetOwner().GetL2Left();

            ISDN::IsdnPacket* pPack = L2.CreatePacket<ISDN::IPacket>();
            std::vector<byte> info(100, 255);
            pPack->AddBack(info);
            L2.DataReq(pPack);
            SwitchState( new WaitFeedData(GetOwner()) );
           
        }*/
        //void DataExchangeTestState::Do()
        //{
        //    Proc();
        //    bool ok = Completed();
        //    if (!ok ) return;

        //    SwitchState( new ReleaseReq(GetOwner()) );
        //}

        //-------------------------------------------------------------

        //void WaitFeedData::Do()
        //{
        //    L3Shell &L3 = GetOwner().GetL3Right();
        //    ISDN::IsdnPacket* pPack = L3.GetLastPacket();
        //    if(!pPack)return;
        //    int len = pPack->GetLenData();
        //    if(len != 100 ) 
        //    {
        //        pPack->Delete();
        //        TUT_ASSERT("WrongPacket in WaitFeedData");
        //    }
        //    for(int i = 0; i < len; i++)
        //        if(pPack->GetByIndex(i) != 255) 
        //        {
        //            pPack->Delete();
        //            TUT_ASSERT("WrongPacket in WaitFeedData");
        //        }
        //    pPack->Delete();
        //    SwitchState( new ReleaseReq(GetOwner()) );
        //}

        //-------------------------------------------------------------

        void ReleaseReq::Do()
        {
            IL3ToL2& l2 = GetOwner().GetL2Left();
            l2.ReleaseReq();
            
            GetOwner().GetL3Right().SetStateWaitRelInd();
            GetOwner().GetL3Left().SetStateWaitRelConf();
            
            SwitchState( new WaitDisconnect(GetOwner()) );
        }

        //-------------------------------------------------------------

        void WaitDisconnect::Do()
        {
            LapdTest &Fsm = GetOwner();
            
            if( !Fsm.GetL3Left().Complete() || !Fsm.GetL3Right().Complete() ) return;
       
            TUT_ASSERT (Fsm.GetL2Left().getL2()->IsST_L2_4());
            TUT_ASSERT (Fsm.GetL2Right().getL2()->IsST_L2_4());
            SwitchState( new EndState(GetOwner()) );
            
        }

        //-------------------------------------------------------------

        void EndState::Do()
        {
            GetOwner().SetComplete();
        }

    } // namespace TestStateImpl

    ITestState* GetStartTest(LapdTest& fsm)  
    {
        return new TestStateImpl::Initial(fsm); 
    };

} // namespace IsdnTest



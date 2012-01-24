
#include "stdafx.h"
// #include "IsdnTestConfig.h"

#include "l3test.h"


namespace IsdnTest
{
	namespace L3TestImpl
    {
        
        
        
        class StartState: public L3State	
	    {	
	    public:	

		    StartState(IL3Test* pL3):L3State(pL3){}	

            void DataInd(ISDN::IPacket *pPacket) // override
            {}

            void OnEmptyEvent(){/* Nothing todo. State remain in force*/}; /*override*/	

	    };

        //------------------

        class WaitEstConf: public L3State	
	    {	
	    public:	

		    WaitEstConf(IL3Test* pL3):L3State(pL3){}	

		    void EstablishConf(); // override

            void OnEmptyEvent() {TUT_ASSERT( 0 && "Empty Event in WaitEstConf" );}/*override*/	

	    };

        //------------------
        class WaitReleaseInd: public L3State
        {
        public:
            WaitReleaseInd(IL3Test* pL3):L3State(pL3){}	

		    void ReleaseInd(); // override

            void OnEmptyEvent() {TUT_ASSERT( 0 && "Empty Event in WaitReleaseInd" );}/*override*/	

        };

        //------------------

        class WaitReleaseConf: public L3State
        {
        public:
            WaitReleaseConf(IL3Test* pL3):L3State(pL3){}	

            void ReleaseConf(); // override

            void OnEmptyEvent() {TUT_ASSERT( 0 && "Empty Event in WaitReleaseConf" );}/*override*/	

        };

        //------------------

        class StateComplete: public L3State	
	    {	
	    public:	

		    StateComplete(IL3Test* pL3):L3State(pL3){m_complete = true;}	

            void OnEmptyEvent() {TUT_ASSERT( 0 && "Empty Event in WaitReleaseConf" );}/*override*/

	    };
        
        //------------------

        void WaitEstConf::EstablishConf() // override 
        {
            //SwitchState( new StateComplete(GetL3()) );
            m_complete = true;
        }

        void WaitReleaseInd::ReleaseInd() // override 
        {
            //SwitchState( new StateComplete(GetL3()) );
            m_complete = true;
        }

        void WaitReleaseConf::ReleaseConf() // override 
        {
            //SwitchState( new StateComplete(GetL3()) );
            m_complete = true;
        }


    }  //namespace L3TestImpl
    
    //-------------------------------------------------

    L3State* L3Test::GetStartTest()
    {
        return new L3TestImpl::StartState(this);
    }

    /*State switchers*/
    void L3Test::SetStateWaitEstConf()
    {
        m_pCurrState->SwitchState( new L3TestImpl::WaitEstConf(this) );
    }

    void L3Test::SetStateWaitRelInd()
    {
        m_pCurrState->SwitchState( new L3TestImpl::WaitReleaseInd(this) );
    }

    void L3Test::SetStateWaitRelConf()
    {
        m_pCurrState->SwitchState( new L3TestImpl::WaitReleaseConf(this) );
    }

};  //namespace IsdnTest


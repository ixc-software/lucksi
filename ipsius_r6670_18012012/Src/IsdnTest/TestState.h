#ifndef TSTATE_H
#define TSTATE_H

#include "stdafx.h"
// #include "IsdnTestConfig.h"
#include "LapdTestClass.h"
#include "Platform/PlatformTypes.h"
#include "Utils/ErrorsSubsystem.h"
#include "logger.h"
#include "ITestState.h"


namespace IsdnTest
{
	//---------------------------------------------------------
	//интерфейс-надстройка над конкретными тестами-состояноями
	
	class TestState: public ITestState
	{
	public:
		
		TestState(LapdTest &ownMachine, const std::string& name, int maxDuration = -1 ):
		m_testFsm(ownMachine),
		m_name(name),
		m_limitTime(maxDuration + ownMachine.getFsm().GetTick()),
		m_testTimeout( (maxDuration<0) ? false : true )
			{}
		
		virtual ~TestState(){}
		
		void Process()
		{
			if ( (m_testTimeout) && (( m_testFsm.getFsm().GetTick() + (dword(0) - m_limitTime) ) < Half)  ) 
            {
                //TODO add kind
                m_testFsm.GetLogger()->Log("Timeout Error in state " + m_name);
                TUT_ASSERT( 0 && "Timeout Error " );
            }
			Do(); // запуск для реализации
		}
		
		std::string GetName()
			{ return m_name; }

	protected:

		virtual void Do() = 0; // перекрыта в реализациях состояний(тестов)
		
		void SwitchState(TestState* newState) // вызов из имплементации сост-теста
		{
			m_testFsm.getFsm().SwitchTo( newState ); // переход машины в новое состояние созданное в импл сост-тест
			delete this; // удаление отработавшего сост-теста
		}
		
		LapdTest& GetOwner()
		{
            return m_testFsm; 
        }

	private:
		
        LapdTest& m_testFsm;
		std::string m_name;
		int m_limitTime;
		static const dword Half = ~dword(0)/2;
		bool m_testTimeout;
	};
}//namespace IsdnTest

#endif

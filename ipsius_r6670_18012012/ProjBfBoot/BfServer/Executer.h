#ifndef __EXECUTER__
#define __EXECUTER__

#include "stdafx.h"

#include "Utils/IBasicInterface.h"


namespace BfServer
{

    class IExecutionUnit : public Utils::IBasicInterface
    {
    public:
        virtual void Execute() = 0;
    };


    //-------------------------------------------------------------------------


    class IExecuter : public Utils::IBasicInterface
    {
    public:
        virtual void RegisterUnit(IExecutionUnit *pUnit) = 0;
        virtual void UnregisterUnit(IExecutionUnit *pUnit) = 0;		
    };


    //-------------------------------------------------------------------------

    
    // –одительский класс дл€ задач Executer-а
    // все унаследованные объекты, должны быть распложены в куче - !!!!
    class ExecutionUnit : public IExecutionUnit
    {
        IExecuter &m_owner;

        virtual void OnExecute() = 0;


    public:
        ExecutionUnit(IExecuter &owner);
        virtual ~ExecutionUnit();


    // IExecutionUnit implementation
    private:
        void Execute();
    };


    //-----------------------------------------------------------------------------


    //  ласс, обеспечивающий регистрацию, дерегистрацию и поочередное выполнение
    // задач
    class Executer : public IExecuter
    {
        std::vector<IExecutionUnit*> m_units;

        bool m_listIsModified;
        bool m_loopBreak;

        void SingleExecutionLoop();

    public:
        Executer();
        ~Executer();

        void Execute();
        void BreakLoop();


    // IExecuter impl
    private:

        void RegisterUnit(IExecutionUnit *pUnit);
        void UnregisterUnit(IExecutionUnit *pUnit);

    };

} // namespace BfServer


#endif

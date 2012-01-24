#ifndef __TIMERBASIC___
#define __TIMERBASIC___


#include "Platform/Platform.h"
#include "Executer.h"


using namespace Platform;


namespace BfServer
{

    class TimerBasic; // Forward declaration of an event generator class


    // ��������� ������� �������
    class ITimerEvents
    {
    public:
        virtual void OnTimer(TimerBasic &sender) = 0;
    };


    //-------------------------------------------------------------------------


    // ����� �������� �������. ����������� ������ � ��������� ����� ����������
    // ������� (� "�����"), � ����� ������������� ������� OnTimer(),
    // �������������� ����� ��������� ������������� ���������
    class TimerBasic : public ExecutionUnit
    {
        ITimerEvents &m_observer;

        bool m_enabled;
        bool m_repeated;

        dword m_startedAt;
        dword m_interval;


    public:
        TimerBasic(IExecuter &owner, ITimerEvents &observer);
        ~TimerBasic();

        void Start(dword interval, bool repeated);
        void Stop();
        bool Enabled() const;


    // ExecutionUnit implementation
    private:
        void OnExecute();
    };

} // namespace BfServer

#endif

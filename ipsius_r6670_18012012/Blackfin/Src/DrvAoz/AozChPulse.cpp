#include "stdafx.h"

#include "Utils/IntToString.h"

#include "AozChPulse.h"

// ----------------------------------------------

namespace DrvAoz
{
        
    void AozChPulse::FsmPulseProcess( bool lineBreaked )
    {
        FsmPulse::FsmState st = m_pulse.State;

        // �������� ���������
        if (st == FsmPulse::St0)
        {
            m_pulse.Clear();
            FsmPulseChangeState(FsmPulse::St23);
            return;
        }

        // �������� -- ����������� ��������
        if (st == FsmPulse::St1)
        {
            if (lineBreaked)
            {
                m_pulse.TBreakMs = 0;
                FsmPulseChangeState(FsmPulse::St11);  // �������
            }
            else
            {
                m_pulse.TCloseMs += m_pulsePeriodMs;
                if (m_pulse.TCloseMs >= m_lineSetup.TMinSerialDial)
                {
                    FsmPulseChangeState(FsmPulse::St12);
                    MsgDialPause();
                }
            }

            return;
        }

        // ������� ����������
        if (st == FsmPulse::St11)
        {
            m_pulse.TBreakMs += m_pulsePeriodMs;

            if (m_pulse.TBreakMs >= m_lineSetup.TProtBounceHookDown)
            {
                if (lineBreaked)
                {
                    m_pulse.TCloseMs = 0;
                    FsmPulseChangeState(FsmPulse::St2);
                    MsgGoesBreaked();
                }
                else
                {
                    m_pulse.TCloseMs += m_pulse.TBreakMs;
                    m_pulse.TBreakMs = 0;
                    FsmPulseChangeState(m_pulse.PrevState);
                    MsgFalseBreakImpulse();
                }
            }

            return;
        }

        // �������� -- �������� �������� �������� ������
        if (st == FsmPulse::St12)
        {
            if (lineBreaked)
            {
                m_pulse.TBreakMs = 0;
                FsmPulseChangeState(FsmPulse::St11);  // �������
            }
            else
            {
                m_pulse.TCloseMs += m_pulsePeriodMs;
                if (m_pulse.TCloseMs >= m_lineSetup.TProtHookUp)
                {
                    FsmPulseChangeState(FsmPulse::St13);
                    MsgHookUp();
                }
            }

            return;
        }

        // ��������� �������� ������
        if (st == FsmPulse::St13)
        {
            if (lineBreaked)
            {
                m_pulse.TBreakMs = 0;
                FsmPulseChangeState(FsmPulse::St11);  // �������
            }
            else
            {
                // nothing, stable
            }

            return;
        }

        // �������� -- ������������ ����������� ������
        if (st == FsmPulse::St2)
        {
            if (lineBreaked)
            {
                m_pulse.TBreakMs += m_pulsePeriodMs;
                if (m_pulse.TBreakMs >= m_lineSetup.TMaxDialPulce)
                {
                    FsmPulseChangeState(FsmPulse::St22);
                    MsgDialPulse();
                }
            }
            else
            {
                m_pulse.TCloseMs = 0;
                FsmPulseChangeState(FsmPulse::St21); // �������
            }

            return;
        }

        // ������� ���������
        if (st == FsmPulse::St21)
        {
            m_pulse.TCloseMs += m_pulsePeriodMs;

            if (m_pulse.TCloseMs >= m_lineSetup.TProtBounceHookUp)
            {
                if (lineBreaked)   // ������ ������������
                {
                    m_pulse.TBreakMs += m_pulse.TCloseMs;
                    m_pulse.TCloseMs = 0;
                    FsmPulseChangeState(m_pulse.PrevState);
                    MsgFalseCloseImpulse();
                }
                else   // ���������� ��������� ���������
                {
                    m_pulse.TBreakMs = 0;
                    FsmPulseChangeState(FsmPulse::St1);
                    MsgGoesClosed();
                }
            }

            return;
        }

        // �������� -- �������� �������� ��������� ������
        if (st == FsmPulse::St22)
        {
            if (lineBreaked)            
            {
                m_pulse.TBreakMs += m_pulsePeriodMs;
                if (m_pulse.TBreakMs >= m_lineSetup.TProtHookDown)
                {
                    FsmPulseChangeState(FsmPulse::St23);
                    MsgHookDown();
                }
            }
            else
            {
                m_pulse.TCloseMs = 0;
                FsmPulseChangeState(FsmPulse::St21); // �������
            }
            
            return;
        }


        // ��������� ��������
        if (st == FsmPulse::St23)
        {
            if (lineBreaked)
            {
                // nothing, stable
            }
            else
            {
                m_pulse.TCloseMs = 0;
                FsmPulseChangeState(FsmPulse::St21);  // �������
            }

            return;
        }

        // ���������� ���
        ESS_HALT("Bad state " + Utils::IntToString(st));
    }

}  // namespace DrvAoz


#ifndef HWBOARDFSMSTATES_H
#define HWBOARDFSMSTATES_H

#include "Utils/FullEnumResolver.h"

namespace IpsiusService
{
    class HwBoardFsmStates : public QObject    
    {
        Q_OBJECT;
        Q_ENUMS(States);           

    public:

        enum States
        { 
            StInitial,          // �������� ���������.
            StDiscovered,       // ����� ���������� 
            StWaitBooterExit,   // �������� �������� booter -> soft, ����� ����� StReady
            StReady,            // ����� ���������� � ������ � �������������
            StOff,              // ����� ���� ����������, ������ �� ��� ����� �� ��������� ���������� (�� ���������? ��� �������? )

            StBusy,             // ����� ����������, �� ��� ������.
            StService,          // ����� ������ �������� HwFinder (���������� �������� etc.)
            StBlocked,          // ����� �������������, ������� �������� � �������� ����� StateDescr
            StWaitBooterEnter,  // �������� �������� soft -> booter, ����� ����� StService 
        };

        QString ResolveState(States val) const
        {
            return Utils::EnumResolve(val).c_str();            
        }        
    
    };

} // namespace IpsiusService

#endif

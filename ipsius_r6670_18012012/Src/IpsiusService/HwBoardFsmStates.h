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
            StInitial,          // Исходное состояние.
            StDiscovered,       // плата обнаружена 
            StWaitBooterExit,   // ожидание перехода booter -> soft, после этого StReady
            StReady,            // плата обнаружена и готова к использованию
            StOff,              // плата была обнаружена, однако от нее давно не приходили броадкасты (ее выключили? она зависла? )

            StBusy,             // Плата обнаружена, но уже занята.
            StService,          // плата занята объектом HwFinder (обновление прошивки etc.)
            StBlocked,          // плата заблокирована, причина доступна в свойстве платы StateDescr
            StWaitBooterEnter,  // ожидание перехода soft -> booter, после этого StService 
        };

        QString ResolveState(States val) const
        {
            return Utils::EnumResolve(val).c_str();            
        }        
    
    };

} // namespace IpsiusService

#endif

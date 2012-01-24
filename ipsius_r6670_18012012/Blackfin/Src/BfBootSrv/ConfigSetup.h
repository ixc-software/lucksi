#ifndef CONFIGSETUP_H
#define CONFIGSETUP_H

#include "RangeIOBase.h"
#include "E1App/NetworkSettings.h"

#include "BfBootCore/ServerTuneParametrs.h"

/*
    Временный инструмент. Гарантирует наличие на флеши некоторых настроек по умолчанию.    
*/

namespace BfBootSrv
{          
    // if clearOther == true  set Default and erase/damage other.       
    void ConfigSetup(RangeIOBase& flash, const BfBootCore::DefaultParam& preset, bool clearOther = true);

    // Позволяет запустить загрузчик в сервисном режиме (для установки фабричных параметров)
    void DeleteDefault(RangeIOBase& flash);

} // namespace BfBootSrv

#endif

#ifndef LAUNCHBOOTER_H
#define LAUNCHBOOTER_H

#include "BooterStartupParams.h"

// запуск загрузчика начинается отсюда

namespace BfBootSrv
{   
	class BoardSetup;

    // Основная версия.
    void LaunchBooter(const BooterStartupParams& params);

    // Версия для эмулятора. Обходит этапы инициализации сетевого стка и восстановления BoardSetup с флеши.
	void LaunchBooter(const BooterStartupParams& params, BoardSetup &boardSetup, bool initNetwork = false);	

} // namespace BfBootSrv

#endif

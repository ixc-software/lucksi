/*  ’од выполнени€ теста: при запущенном Yate - сервере запускаетс€ тест, после чего должен подн€тс€ D-канал.
*   „ерез Yate - выполн€етс€ вызов ноера 37190000. Dss система собранна€ в тесте принимает вызов и устанавливает 
*   соединение. „ерез некоторое врем€ завершает вызов и создает исход€щий на номер 7190001. Ётот вызов нужно 
*   вручную прин€ть на Yate. ƒолжно установитс€ соединение. “ест завершаетс€ после его разрыва состороны Yate.
*/
#ifndef YATETESTL2_H
#define YATETESTL2_H

#include "Utils/ExeName.h"
#include "iLog/LogWrapper.h"

#include "YateL2TestObj.h"
#include "YateL3TestObj.h"

#include "Domain/DomainTestRunner.h"

namespace IsdnTest
{
    static bool YateTestL2()
    {
        iLogW::LogStore logStore;      

        YateL2TestProfile prof(logStore);

        Domain::DomainTestRunner test(-1);
        return test.Run<YateL2TestObj>( prof );                
    }

    static bool YateTestL3()
    {
        iLogW::LogStore logStore;      

        YateL3TestProfile prof(logStore);

        Domain::DomainTestRunner test(-1);
        return test.Run<YateL3TestObject>( prof );
        
    }
} //namespace IsdnTest

#endif

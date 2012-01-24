#ifndef CFGRESTOREERROR_H
#define CFGRESTOREERROR_H

#include "Utils/ErrorsSubsystem.h"

namespace BfBootSrv
{

    // ошибки востановления Config из потока данных
    ESS_TYPEDEF(CfgRestoreError);

    //ESS_TYPEDEF_FULL(ErrCfgReadStream, CfgRestoreError);   // ошибка при чтении из потока
    ESS_TYPEDEF_FULL(ErrCfgNoParam, CfgRestoreError);      // в потоковых данных нет описания одного из полей Config
    ESS_TYPEDEF_FULL(ErrCfgCRC, CfgRestoreError);          // ошибка при проверке CRC
    ESS_TYPEDEF_FULL(ErrCfgVersion, CfgRestoreError);      // версия данных не соответствует версии кода Config
    ESS_TYPEDEF_FULL(ErrCfgParse, CfgRestoreError);      // версия данных не соответствует версии кода Config
    ESS_TYPEDEF_FULL(ErrCantConvertVal, CfgRestoreError);      // версия данных не соответствует версии кода Config

} // namespace BfBootSrv

#endif

#ifndef CFGRESTOREERROR_H
#define CFGRESTOREERROR_H

#include "Utils/ErrorsSubsystem.h"

namespace BfBootSrv
{

    // ������ ������������� Config �� ������ ������
    ESS_TYPEDEF(CfgRestoreError);

    //ESS_TYPEDEF_FULL(ErrCfgReadStream, CfgRestoreError);   // ������ ��� ������ �� ������
    ESS_TYPEDEF_FULL(ErrCfgNoParam, CfgRestoreError);      // � ��������� ������ ��� �������� ������ �� ����� Config
    ESS_TYPEDEF_FULL(ErrCfgCRC, CfgRestoreError);          // ������ ��� �������� CRC
    ESS_TYPEDEF_FULL(ErrCfgVersion, CfgRestoreError);      // ������ ������ �� ������������� ������ ���� Config
    ESS_TYPEDEF_FULL(ErrCfgParse, CfgRestoreError);      // ������ ������ �� ������������� ������ ���� Config
    ESS_TYPEDEF_FULL(ErrCantConvertVal, CfgRestoreError);      // ������ ������ �� ������������� ������ ���� Config

} // namespace BfBootSrv

#endif

#ifndef CONFIGSETUP_H
#define CONFIGSETUP_H

#include "RangeIOBase.h"
#include "E1App/NetworkSettings.h"

#include "BfBootCore/ServerTuneParametrs.h"

/*
    ��������� ����������. ����������� ������� �� ����� ��������� �������� �� ���������.    
*/

namespace BfBootSrv
{          
    // if clearOther == true  set Default and erase/damage other.       
    void ConfigSetup(RangeIOBase& flash, const BfBootCore::DefaultParam& preset, bool clearOther = true);

    // ��������� ��������� ��������� � ��������� ������ (��� ��������� ��������� ����������)
    void DeleteDefault(RangeIOBase& flash);

} // namespace BfBootSrv

#endif

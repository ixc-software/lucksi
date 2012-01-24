/*  ��� ���������� �����: ��� ���������� Yate - ������� ����������� ����, ����� ���� ������ �������� D-�����.
*   ����� Yate - ����������� ����� ����� 37190000. Dss ������� ��������� � ����� ��������� ����� � ������������� 
*   ����������. ����� ��������� ����� ��������� ����� � ������� ��������� �� ����� 7190001. ���� ����� ����� 
*   ������� ������� �� Yate. ������ ����������� ����������. ���� ����������� ����� ��� ������� ��������� Yate.
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

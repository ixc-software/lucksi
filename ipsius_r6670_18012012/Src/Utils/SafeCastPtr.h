#ifndef __SAFECASTPTR__
#define __SAFECASTPTR__

#include "ErrorsSubsystem.h"

namespace Utils
{

    // ����������� p � T* ����� dynamic_cast, ���� �������������� �� ������� -- ESS_ASSERT
    template<class T, class U>
    T* SafeCastPtr(U *p)
    {
        T* pRes = dynamic_cast<T*>(p);
        ESS_ASSERT(pRes != 0);
        return pRes;
    }

};

#endif


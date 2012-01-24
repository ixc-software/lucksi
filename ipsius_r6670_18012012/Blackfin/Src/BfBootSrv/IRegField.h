#ifndef IREGFIELD_H
#define IREGFIELD_H

#include "Utils/IBasicInterface.h"

namespace BfBootSrv
{
    class ICfgField;

    // ��������� ����������� Field � FieldList
    class IRegField : public Utils::IBasicInterface
    {
    public:
        virtual void RegField(ICfgField& field) = 0;
    };
} // namespace BfBootSrv

#endif

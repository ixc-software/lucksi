#include "stdafx.h"

#include "DRI/NamedObjectFactory.h"
#include "NamedObjectTestType.h"

namespace
{

    DRI::NamedObjectTypeRegister<DriTests::NamedObjectTestType> GReg;

}  // namespace

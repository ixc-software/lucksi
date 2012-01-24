#ifndef __EXTPROPERTY__

#define  __EXTPROPERTY__

#include <stdio.h>
#include <stdlib.h>
#include "moc.h"

namespace MocExt
{

    bool PrintReadProperty(FILE *out, ClassDef *cdef, int propindex,const PropertyDef &p);
    bool PrintWriteProperty(FILE *out, ClassDef *cdef, int propindex,const PropertyDef &p);
}; // namespace MocExt

#endif


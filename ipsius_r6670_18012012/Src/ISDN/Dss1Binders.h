#ifndef DSS1BINDERS_H
#define DSS1BINDERS_H

#include "ObjLink/ObjectLinkBinder.h"

namespace ISDN
{
    class IL2ToDriver;
    class IDriverToL2;
    class IL2ToL3;
    class IL3ToL2;

    typedef boost::shared_ptr< ObjLink::ObjLinkBinder<IL2ToDriver> > BinderToIDriver;
    typedef boost::shared_ptr< ObjLink::ObjLinkBinder<IDriverToL2> > BinderToIDrvToL2;

    typedef boost::shared_ptr< ObjLink::ObjLinkBinder<IL2ToL3> > BinderToIL2ToL3;
    typedef boost::shared_ptr< ObjLink::ObjLinkBinder<IL3ToL2> > BinderToIL3ToL2;

} // namespace ISDN

#endif

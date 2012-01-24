/* =============================================================================
 *
 *  Description: This is a C++ header file for Thread base
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _base_H_    
#define _base_H_

#pragma diag(push)
#pragma diag(suppress: 177,401,451,826,831,1462)

#include "VDK.h"
   
#pragma diag(pop)

class base : public VDK::Thread 
{
//	static base *m_this;
public:
    base(VDK::Thread::ThreadCreationBlock&);
    virtual ~base();
    virtual void Run();
    virtual int ErrorHandler();
    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock&);
};

#endif /* _base_H_ */

/* ========================================================================== */

#ifndef IL3CALLS
#define IL3CALLS

#include "stdafx.h"
#include "Utils/IBasicInterface.h"
#include "CallRef.h"
#include "SetBCannels.h"


namespace ISDN
{
    class L3Call;

    // Внутренний интерфейс владельца списка вызовов
    class IL3Calls : public Utils::IBasicInterface
    {
    public:
        /*return 0 if not found*/
        virtual L3Call* Find(const CallRef&) = 0;

        virtual L3Call* CreateInCall(const CallRef&) = 0;
        virtual void AsyncDeleteCall(L3Call* pL3Call) = 0;              
    };

} // ISDN

#endif


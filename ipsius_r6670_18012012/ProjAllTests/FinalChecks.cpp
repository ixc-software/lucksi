#include "stdafx.h"
#include "FinalChecks.h"
#include "iCore/MsgBase.h"

// --------------------------------------

void FinalChecks()
{

    try
    {
        iCore::MsgBase::CounterCheck();
    }
    catch (/*const*/ std::exception& ex)
    {
        std::cerr << "FinalChecks: " << ex.what() << std::endl;
    }
       
}

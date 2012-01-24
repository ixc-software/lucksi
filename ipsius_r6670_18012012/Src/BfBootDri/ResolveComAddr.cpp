#include "stdafx.h"
#include "ResolveComAddr.h"

namespace BfBootDRI
{
    bool ResolveComAddr(QString addr, int& num, int& rate)
    {
        QStringList fullList = addr.split("COM");
        if (fullList.size() != 2 || !fullList[0].isEmpty()) return false;

        QStringList list = fullList[1].split("@");    
        if (list.size() != 2) return false;

        bool ok = false;     
        num = list[0].toInt(&ok);        
        if (!ok) return false;
        rate = list[1].toInt(&ok);
        return ok;
    }
} // namespace BfBootDri

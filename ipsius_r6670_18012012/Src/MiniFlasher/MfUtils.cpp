
#include "stdafx.h"
#include "Utils/QtHelpers.h"

#include "MfUtils.h"

namespace MiniFlasherUtils
{
    QString NumberFormat(int64 number)
    {
        return Utils::NumberFormat(number);
    }

    // --------------------------------------------------------------------------------
    
    QString OperationInfo(const QString &opName, dword opTimeMs)
    {
        double time = opTimeMs / 1000.0;
        
		return QString("%1 is completed. Time %2 sec.")
			.arg(opName).arg(time,  0, 'f', 1);
    }

    // --------------------------------------------------------------------------------	

	QString OperationInfo(const QString &opName, dword totalBytes, dword opTimeMs)
	{
		double time = opTimeMs / 1000.0;
		if (time <= 0) time = 0.001;  // fix dev by zero

        QString speedDesc("Kb/sec");
		double speed = (totalBytes / 1024) / time;
        if (speed == 0) 
        {
            speed = totalBytes / time;
            speedDesc = "b/sec";
        }

		return QString("%1 is completed. Time %2 sec, speed %3 %4.")
			.arg(opName)
			.arg(time,  0, 'f', 1)
			.arg(speed, 0, 'f', 1)
            .arg(speedDesc);
	}

    // --------------------------------------------------------------------------------

    QString OperationHead(const QString &opName, int comPort, int baudRate,
                          const QString &fileName, dword totalBytes)
    {
        
        QString res = QString("%1 started.").arg(opName);

        if (!fileName.isEmpty()) res += QString(" File: '%1'.").arg(fileName);
        if (totalBytes != 0) res += QString(" Size: %1.").arg(NumberFormat(totalBytes));
        
        res += QString(" Serial port #%1, baud rate: %2.")
                       .arg(comPort).arg(baudRate);

        return res;
    }

    // --------------------------------------------------------------------------------

    
} // namespace MiniFlasherUtils

#include "stdafx.h"
#include "Utils/IntToString.h"
#include "LogBinaryUtils.h"

namespace 
{
    std::string LogData(const void *data, int dataSize, int outputSize, const std::string &postfix = "")
    {
        std::ostringstream out;
        out << " [bin " << dataSize << "] ";
       
		if (data == 0) return "";
	
		if(outputSize)
        {
			if(outputSize > dataSize) outputSize = dataSize;

            out << Utils::DumpToHexString(data, outputSize);
        }
		if(outputSize < dataSize) out << postfix;
        
        return out.str();
    }
};

namespace iLogW
{
    std::string LogBinaryAll(const void *data, int size)
    {
        return LogData(data, size, size);
    }

    std::string LogBinaryShort(const void *data, int size, int maxSize)
    {
		if(maxSize == -1) maxSize = size;

        return LogData(data, size, maxSize, " (...)");
    }

    std::string LogBinaryLarge(const void *data, int size, int minSize)
    {
        return (size >= minSize) ? LogData(data, size, size) : "";
    }
};


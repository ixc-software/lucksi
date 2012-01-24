#ifndef _LOG_BINARY_H_
#define _LOG_BINARY_H_

#include "stdafx.h"
#include "Utils/IBasicInterface.h"

namespace iLogW
{
    std::string LogBinaryAll(const void *data, int size);
    std::string LogBinaryShort(const void *data, int size, int maxSize);
    std::string LogBinaryLarge(const void *data, int size, int minSize);
};

#endif


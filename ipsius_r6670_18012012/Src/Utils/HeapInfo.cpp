#include "stdafx.h"
#include "Platform/Platform.h"
#include "QtHelpers.h"
#include "HeapInfo.h"

namespace Utils
{
    std::string GetHeapInfo(size_t *pAllocated)
    {
        Platform::HeapState hs = Platform::GetHeapState();

        if (pAllocated != 0) *pAllocated = hs.BytesAllocated;

        if (!hs.Availble()) return "Heap: <no info>";

		std::ostringstream out;
		out << "Heap: blocks "
			<< NumberFormat(hs.BlockCount)
			<< ", allocated %2"
			<< NumberFormat(hs.BytesAllocated);
		return out.str();
    }

};



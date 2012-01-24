#ifndef IBIDIRBUFFCREATOR_H
#define IBIDIRBUFFCREATOR_H

#include "IBasicInterface.h"

namespace Utils
{
    class BidirBuffer;

    class IBidirBuffCreator : public IBasicInterface
    {
    public:
        // can throw NoBlocksAvailable
        virtual BidirBuffer* CreateBidirBuff(int offset) = 0;   // alloc with custom offset
        virtual BidirBuffer* CreateBidirBuff() = 0;             // alloc with default offset
    };
} // namespace Utils

#endif


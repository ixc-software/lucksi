#ifndef _IFINISH_TCP_TEST_H_
#define _IFINISH_TCP_TEST_H_

#include "stdafx.h"

namespace iNetTests
{

    class IFinish : public Utils::IBasicInterface 
    {
    public:
        class Item : public boost::noncopyable
        {
        public:
            virtual ~Item(){} 
            bool IsSame(Item *item) const { return item == this; }
        };
        virtual void Finish(Item  *ipem) = 0;
    };
};

#endif

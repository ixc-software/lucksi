#ifndef WRONGL3PACKETTEST_H
#define WRONGL3PACKETTEST_H

#include "Domain/DomainClass.h"
#include "ITestResult.h"
#include "NObjWrongL3PacketTestSettings.h"

namespace IsdnTest
{
    // тест обработки ошибок при парсинге битых пакетов
    class WrongL3PacketTest
    {
        class Impl;
        boost::scoped_ptr<Impl> m_impl;
    public:

        typedef NObjWrongL3PacketTestSettings TNObjProfile;

        WrongL3PacketTest(Domain::DomainClass &domain, const TNObjProfile& prof, ITestResult& callback);
        void AbortTest();
        ~WrongL3PacketTest();

    };
} // namespace IsdnTest

#endif

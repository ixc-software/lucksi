
#include "stdafx.h"

#include "qchar.h"
#include "QtCoreTests.h"
#include "Utils/ErrorsSubsystem.h"

/*
    TODO: 
        - Tests for unicode symbols
*/

namespace
{
    void TestConstructors()
    {
        TUT_ASSERT(QChar().unicode() == 0);

        TUT_ASSERT(QChar('A').unicode() == 65); 

        uchar uc = 49; // '1'
        TUT_ASSERT(QChar(uc).unicode() = 49);

        TUT_ASSERT(QChar(300, 0).unicode() == 44);
        TUT_ASSERT(QChar(65, 0).unicode() == 65);
        TUT_ASSERT(QChar(0, 1).unicode() == 256);
        
        ushort us = 66;
        TUT_ASSERT(QChar(us) == QChar('B'));

        short s = 67;
        TUT_ASSERT(QChar(s) == QChar('C'));

        uint ui = 68;
        TUT_ASSERT(QChar(ui) == QChar('D'));

        int i = 69;
        TUT_ASSERT(QChar(i) == QChar('E'));

        QChar::SpecialCharacter lineSep = QChar::LineSeparator;
        TUT_ASSERT(QChar(lineSep).unicode() == QChar::LineSeparator);
    }

    // -----------------------------------------------

    void TestAsciiConvertion()
    {
        char ch = 'A';
        ushort chCode = (ushort)ch;
        TUT_ASSERT(QChar(ch).toAscii() == ch);
        TUT_ASSERT(QChar(ch).toLatin1() == ch);
        TUT_ASSERT(QChar::fromAscii(ch).unicode() == chCode);
        TUT_ASSERT(QChar::fromLatin1(ch).unicode() == chCode);

        ushort uniCh = 2205;
        TUT_ASSERT(QChar(uniCh).toAscii() == '\0');
        TUT_ASSERT(QChar(uniCh).toLatin1() == '\0');
    }

    // -----------------------------------------------

    void TestOperators()
    {
        QChar ch1('A');
        QChar ch2(ch1);
        QChar ch3('B');

        TUT_ASSERT(ch1 == ch2);
        TUT_ASSERT(ch1 != ch3);
        TUT_ASSERT(ch1 <= ch2);
        TUT_ASSERT(ch1 <= ch3);
        TUT_ASSERT(ch3 >= ch1);
        TUT_ASSERT(ch1 >= ch2);
        TUT_ASSERT(ch1 < ch3);
        TUT_ASSERT(ch3 > ch2);
    }
    
    // -----------------------------------------------

    void TestOther()
    {
        ushort uniCh = 49;
        QChar qCh(uniCh);
        TUT_ASSERT(qCh.unicode() == uniCh);

        ++uniCh;
        qCh.unicode() = uniCh;
        TUT_ASSERT(qCh.unicode() == uniCh);
    
        TUT_ASSERT(QChar('A').toLower() == QChar('a'));
        TUT_ASSERT(QChar('a').toLower() == QChar('a'));
        TUT_ASSERT(QChar('1').toLower() == QChar('1'));

        TUT_ASSERT(QChar('1').toUpper() == QChar('1'));
        TUT_ASSERT(QChar('a').toUpper() == QChar('A'));
        TUT_ASSERT(QChar('A').toUpper() == QChar('A'));

        ushort spaceCh = 9;
        while (spaceCh <= 13)
        {
            TUT_ASSERT(QChar(spaceCh).isSpace());
            ++spaceCh;
        }
    }
    
} // namespace 

// -------------------------------------------

namespace QtCoreTests
{
    void QCharTests()
    {
        TestConstructors();
        TestOperators();
        TestAsciiConvertion();
        TestOther();
    }
    
} // namespace QtCoreTests




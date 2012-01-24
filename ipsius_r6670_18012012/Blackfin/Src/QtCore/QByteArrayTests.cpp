
#include "stdafx.h"

#include "qbytearray.h"
#include "QtCoreTests.h"
#include "Utils/ErrorsSubsystem.h"

// test helpers
namespace 
{
    void Check(const QByteArray &toCheck, const std::string &data)
    {
        TUT_ASSERT(std::string( toCheck.data() ) == data);
    }
    
} // namespace 

// --------------------------------------------------------------

// tests
namespace
{
    /*
    QByteArray();
    QByteArray(const char *data);
    QByteArray(const char *data, int size);
    QByteArray(int size, char c);
    QByteArray(const QByteArray &other);
    */
    
    void TestConstructors()
    {
        std::string data;
        TUT_ASSERT(QByteArray().size() == data.size());
        
        data = "abcdef";
        Check(QByteArray (data.c_str() ), data);
        
        Check(QByteArray( data.c_str(), data.size() ), data);
        Check(QByteArray( QByteArray( data.c_str() ) ), data);
        
        char ch = 'a';
        int size = 4;
        Check( QByteArray(size, ch), std::string(size, ch) );
    }

    // -------------------------------------------------------------

    /*
    int size() const;
    bool isEmpty() const;
    void resize(int size);
    int capacity() const;
    void reserve(int size);
    int count() const;
    int length() const;
    bool isNull() const;
    void clear();
    */
    
    void TestUtilites()
    {
        std::string data;
        QByteArray test;
        TUT_ASSERT( test.isNull() );
        TUT_ASSERT( (test.isEmpty() == data.empty()) && (test.isEmpty()) );
        TUT_ASSERT(test.size() == 0);

        test.resize(4);
        TUT_ASSERT(test.size() == 4);
        
        data = "abcdef";
        QByteArray test2(data.c_str());
        TUT_ASSERT(test2.size() == data.size());
        TUT_ASSERT(test2.length() == data.length());
        TUT_ASSERT(test2.count() == data.size());

        data.clear();
        test2.clear();
        TUT_ASSERT(test2.isEmpty());
        test2.reserve(4);
        TUT_ASSERT(test2.capacity() == 4);
    }

    // -------------------------------------------------------------

    /*
    QByteArray& operator=(const QByteArray &other);
    QByteArray& operator=(const char *str);
    
    QByteArray& operator+=(char c);
    QByteArray& operator+=(const char *s);
    QByteArray& operator+=(const QByteArray &a);

    bool operator==(const QByteArray &a1, const QByteArray &a2);
    bool operator==(const QByteArray &a1, const char *a2);
    bool operator==(const char *a1, const QByteArray &a2);

    bool operator!=(const QByteArray &a1, const QByteArray &a2);
    bool operator!=(const QByteArray &a1, const char *a2);
    bool operator!=(const char *a1, const QByteArray &a2);

    bool operator<(const QByteArray &a1, const QByteArray &a2);
    bool operator<(const QByteArray &a1, const char *a2);
    bool operator<(const char *a1, const QByteArray &a2);
    */
    
    void TestOperators()
    {
        std::string data("abcdef");
        QByteArray ba(data.c_str());
        QByteArray copyBa = ba;
        TUT_ASSERT(copyBa == ba);

        ba = data.c_str();
        TUT_ASSERT(ba == data.c_str());

        data += '.';
        ba += '.';
        TUT_ASSERT(ba == data.c_str());

        data += "qwerty";
        ba += "qwerty";
        TUT_ASSERT(data.c_str() == ba);

        ba.clear();
        ba += copyBa;
        TUT_ASSERT(ba == copyBa);

        TUT_ASSERT( QByteArray("abc") < QByteArray("abcd") );
        TUT_ASSERT( QByteArray("abc") < "abcdef" );
        TUT_ASSERT( "abc" < QByteArray("abcdef") );

        TUT_ASSERT( QByteArray("abc") != QByteArray("abcd") );
        TUT_ASSERT( QByteArray("abc") != "abcdef" );
        TUT_ASSERT( "abc" != QByteArray("abcdef") );
    }

    // -------------------------------------------------------------

    /*
    int indexOf(char c, int from = 0) const;
    int indexOf(const char *c, int from = 0) const;
    int indexOf(const QByteArray &a, int from = 0) const;

    int lastIndexOf(char c, int from = -1) const;
    int lastIndexOf(const char *c, int from = -1) const;
    int lastIndexOf(const QByteArray &a, int from = -1) const;
    
    QBool contains(char c) const;
    QBool contains(const char *a) const;
    QBool contains(const QByteArray &a) const;

    QByteArray left(int len) const;
    QByteArray right(int len) const;
    QByteArray mid(int index, int len = -1) const;

    bool startsWith(const QByteArray &a) const;
    bool startsWith(char c) const;
    bool startsWith(const char *c) const;

    bool endsWith(const QByteArray &a) const;
    bool endsWith(char c) const;
    bool endsWith(const char *c) const;
    */

    void TestFind()
    {
        QByteArray ba("sticky question");
        QByteArray toFind("sti");
        TUT_ASSERT(ba.indexOf(toFind) == 0);
        TUT_ASSERT(ba.indexOf(toFind, 1) == 10);
        TUT_ASSERT(ba.indexOf('s', 10) == 10);
        TUT_ASSERT(ba.indexOf(toFind.data(), 11) < 0);

        TUT_ASSERT(ba.lastIndexOf(toFind) == 10);
        TUT_ASSERT(ba.lastIndexOf(toFind, 13) == 10);
        TUT_ASSERT(ba.lastIndexOf(toFind, 5) == 0);
        TUT_ASSERT(ba.lastIndexOf('x', 8) < 0);

        TUT_ASSERT(ba.contains('s'));
        TUT_ASSERT(ba.contains(toFind.data()));
        TUT_ASSERT(ba.contains(toFind));
        TUT_ASSERT(!ba.contains("qwscrf"));

        TUT_ASSERT(ba.left(3) == toFind);
        TUT_ASSERT(ba.left(ba.size() + 4) == ba);
        
        TUT_ASSERT(ba.right(3) == "ion");
        TUT_ASSERT(ba.right(ba.size() + 4) == ba);

        TUT_ASSERT(ba.mid(10, 3) == toFind);
        TUT_ASSERT(ba.mid(10, ba.size() + 4) == "stion");

        TUT_ASSERT(ba.startsWith(toFind));
        TUT_ASSERT(ba.startsWith('s'));
        TUT_ASSERT(ba.startsWith(toFind.data()));

        TUT_ASSERT(ba.endsWith("ion"));
        TUT_ASSERT(ba.endsWith('n'));
        TUT_ASSERT(ba.endsWith(QByteArray("ion")));
    }

    // -------------------------------------------------------------

    /*
    QByteArray& fill(char c, int size = -1);
    QByteArray& append(char c);
    QByteArray& append(const char *s);
    QByteArray& append(const QByteArray &a);
    
    QByteArray& insert(int i, char c);
    QByteArray& insert(int i, const char *s);
    QByteArray& insert(int i, const QByteArray &a);

    void push_back(char c);
    void push_back(const char *c);
    void push_back(const QByteArray &a);
    */
    
    void TestAddInsert()
    {
        std::string data(15, 'c');
        TUT_ASSERT(QByteArray().fill('c', 15) == data.c_str());
        TUT_ASSERT(QByteArray("abcdef").fill('x', 1) == "x");

        TUT_ASSERT(QByteArray("ab").append('c') == "abc");
        TUT_ASSERT(QByteArray("abc").append("de") == "abcde");
        TUT_ASSERT(QByteArray("abc").append(QByteArray("def")) == "abcdef");

        TUT_ASSERT(QByteArray("aef").insert(1, QByteArray("bcd")) == "abcdef");
        TUT_ASSERT(QByteArray("abc").insert(3, 'd') == "abcd");
        TUT_ASSERT(QByteArray("abc").insert(3, "de") == "abcde");

        QByteArray test("ab");
        test.push_back('c');
        TUT_ASSERT(test == "abc");
        test.push_back("de");
        TUT_ASSERT(test == "abcde");
        test.push_back(QByteArray("f"));
        TUT_ASSERT(test == "abcdef");
    }

    // -------------------------------------------------------------

    /*
    const char* data() const;
    inline const char* constData() const;

    const char at(int i) const;
    const char operator[](int i) const;
    const char operator[](uint i) const;
    
    const_iterator begin() const;
    const_iterator constBegin() const;
    
    const_iterator end() const;
    const_iterator constEnd() const;
    */
    
    void TestDataAccess()
    {
        std::string data("abc");
        QByteArray test(data.c_str());
        TUT_ASSERT(*test.data() == *data.c_str());
        TUT_ASSERT(*test.constData() == *data.c_str());

        TUT_ASSERT(test.at(1) == 'b');
        test[1] = 'x';
        TUT_ASSERT(test[1] == 'x');

        TUT_ASSERT(*test.begin() == 'a');
        TUT_ASSERT(*test.constBegin() == 'a');

        TUT_ASSERT(*(test.end() - 1) == 'c');
        TUT_ASSERT(*(test.constEnd() - 1) == 'c');

        // char* data();
        // iterator begin();
        // iterator end();
    }

    // -------------------------------------------------------------

    /*
    void truncate(int pos);
    void chop(int n);
    QByteArray trimmed() const;

    QByteArray& remove(int index, int len);

    QByteArray& replace(int index, int len, const char *s);
    QByteArray& replace(int index, int len, const QByteArray &s);
    */
    
    void TestRemoveReplace()
    {
        QByteArray ba(" ab c d  \t \t \t");
        ba = ba.trimmed();
        TUT_ASSERT(ba == "ab c d");

        ba.chop(2);
        TUT_ASSERT(ba == "ab c");

        ba.truncate(2);
        TUT_ASSERT(ba == "ab");

        TUT_ASSERT(QByteArray("adef").replace(0, 1, "abc") == "abcdef");
        TUT_ASSERT(QByteArray("adef").replace(0, 8, "abc") == "abc");
        TUT_ASSERT(QByteArray("aaaadef").replace(0, 4, QByteArray("abc")) == "abcdef");

        TUT_ASSERT(QByteArray("abcd").remove(1, 2) == "ad");
        TUT_ASSERT(QByteArray("abcd").remove(2, -1) == "ab");
        TUT_ASSERT(QByteArray("abcd").remove(2, 8) == "ab");
    }
    
} // namespace

// -------------------------------------------------

namespace QtCoreTests
{
    void QByteArrayTests()
    {
        TestConstructors();
        TestUtilites();
        TestOperators();
        TestFind();
        TestAddInsert();
        TestDataAccess();
        TestRemoveReplace();
    }
    
} // namespace QtCoreTests

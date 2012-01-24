
#include "stdafx.h"

#include "qstring.h"
#include "QtCoreTests.h"


/*
    TODO:
        - TestConvertion() with unicode
        - check numeric convertion with base-parameter

*/

// test helpers
namespace 
{
    void Check(const QString &toCheck, const std::wstring &data)
    {
        TUT_ASSERT( toCheck.size() == data.size() );
        for (int i = 0; i < toCheck.size(); ++i)
        {
            TUT_ASSERT( toCheck.at(i).unicode() == data.at(i) );
        }
    }

    // ---------------------------------------------------------------

    // test1 < test2
    template<class T>
    void CheckQStringCompareOperators(const T &t1, const T &t2)
    {
        QString test1(t1);
        QString test2(t2);
        
        TUT_ASSERT(test1 == t1);
        TUT_ASSERT(test1 != t2);
        TUT_ASSERT(test1 < t2);
        TUT_ASSERT(test2 > t1);
        TUT_ASSERT(test1 <= t1);
        TUT_ASSERT(test1 <= t2);
        TUT_ASSERT(test2 >= t2);
        TUT_ASSERT(test2 >= t1);
    }
    
} // namespace

// ---------------------------------------------------------------

// tests
namespace
{
    /*
    QString();
    QString(const QChar *unicode, int size);
    QString(QChar c);
    QString(int size, QChar c);
    QString(const QString &other);
    QString(const char *ch);
    QString(const QByteArray &a);
    QString(const Null &);
    */
    
    void TestConstructors()
    {
        std::wstring data;
        Check(QString(), data);
        Check(QString( QString::Null() ), data);

        char ch = 'c';
        wchar_t wch = 2025;
        TUT_ASSERT( QString(ch) == QChar(ch) );
        TUT_ASSERT( QString( QChar(wch) ) == QChar(wch) );

        QString test("abc");
        TUT_ASSERT(test == "abc");
        TUT_ASSERT(test.size() == 3);

        TUT_ASSERT(QString(test) == test);

        TUT_ASSERT( test == QByteArray("abc") );

        Check(QString( 4, QChar(ch) ), std::wstring(4, ch) );
        Check(QString( 4, QChar(wch) ), std::wstring(4, wch) );

        QChar str[3] = {'a', 'b', 'c'};
        TUT_ASSERT(QString(str, 3) == "abc");
    }

    // ---------------------------------------------------------------

    /*
    QString& operator=(QChar c);
    QString& operator=(const QString &other);
    QString& operator=(const char *ch);
    QString& operator=(char c);
    QString& operator=(const QByteArray &a);
    QString &operator=(const Null &);
    */
    
    void TestCopyAssignment()
    {
        char ch = 'c';
        wchar_t wch = 2025;

        QString test = QChar(ch);
        TUT_ASSERT( test == QChar(ch) );
        test = QChar(wch);
        TUT_ASSERT( test == QChar(wch) );

        test = ch;
        TUT_ASSERT( test == QChar(ch) );
        
        QString test2 = test;
        TUT_ASSERT(test2 == test);

        test = "abc";
        TUT_ASSERT(test == "abc");

        QByteArray ba("abc");
        test = ba;
        TUT_ASSERT(test == ba);

        test = QString::Null();
        TUT_ASSERT( test.isNull() );
    }

    // ---------------------------------------------------------------

    /*
    int size() const;
    int count() const;
    int length() const;
    bool isEmpty() const;
    bool isNull() const;
    void resize(int size);
    int capacity() const;
    void reserve(int size);
    QString toLower() const;
    QString toUpper() const;
    */

    void TestUtilities()
    {
        std::string data;
        QString test;
        TUT_ASSERT( test.isNull() );
        TUT_ASSERT( (test.isEmpty() == data.empty()) && (test.isEmpty()) );
        TUT_ASSERT(test.size() == 0);

        test.resize(4);
        TUT_ASSERT(test.size() == 4);
        
        data = "abcdef";
        QString test2( data.c_str() );
        TUT_ASSERT( test2.size() == data.size() );
        TUT_ASSERT( test2.length() == data.length() );
        TUT_ASSERT( test2.count() == data.size() );

        data.clear();
        test2.clear();
        TUT_ASSERT(test2.isEmpty());

        test2.reserve(4);
        TUT_ASSERT(test2.capacity() == 4);
        
        TUT_ASSERT(QString("aBcdEf").toLower() == "abcdef");
        TUT_ASSERT(QString("aBcdEf").toUpper() == "ABCDEF"); 
    }

    // ---------------------------------------------------------------

    /*
    const QChar at(int i) const;

    wchar_t& operator[](int i);
    wchar_t& operator[](uint i);

    const wchar_t* data() const;
    const wchar_t* unicode() const;
    const wchar_t* constData() const;
    
    const_iterator begin() const;
    const_iterator constBegin() const;
    const_iterator end() const;
    const_iterator constEnd();
    */
    
    void TestDataAccess()
    {
        std::string str("abcd");
        QString qStr( str.c_str() );
        int i = 2;
        uint ui = 3;
        
        TUT_ASSERT( QChar( str.at(i) ) == qStr.at(i) );
        TUT_ASSERT( QChar( str[i] ) == qStr[i] );
        TUT_ASSERT( QChar( str[ui] ) == qStr[ui] );

        qStr[0] = 'x';
        TUT_ASSERT( qStr[0] == 'x');

        // iterators
        wchar_t wchars[4] = {'a', 'b', 'c', 'd'};
        std::wstring wstr(wchars, 4);
        qStr = QString::fromStdWString(wstr);
        
        TUT_ASSERT(std::wstring( qStr.data() ) == wstr);
        TUT_ASSERT(std::wstring( qStr.constData() ) == wstr);
        TUT_ASSERT(std::wstring( qStr.unicode() ) == wstr);
        TUT_ASSERT(std::wstring( qStr.begin() ) == wstr);    
        TUT_ASSERT(std::wstring( qStr.constBegin() ) == wstr);
        TUT_ASSERT(*(qStr.end() - 1) == 'd');
        TUT_ASSERT(*(qStr.constEnd() - 1) == 'd');
    }

    // ---------------------------------------------------------------

    /*
    int indexOf(QChar c, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int indexOf(const QString &s, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    int lastIndexOf(QChar c, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int lastIndexOf(const QString &s, int from = -1, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
   
    QBool contains(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    QBool contains(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int count(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    int count(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    QString left(int n) const;
    QString right(int n) const;
    QString mid(int position, int n = -1) const;

    bool startsWith(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool startsWith(const QChar &c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;

    bool endsWith(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    bool endsWith(const QChar &c, Qt::CaseSensitivity cs = Qt::CaseSensitive) const;
    */
    
    void TestFind()
    {
        QString str("sticky question");
        QString toFind("sti");
        QString toFindInCase("StI");
        
        // position
        TUT_ASSERT(str.indexOf(toFind) == 0);
        TUT_ASSERT(str.indexOf(toFind, 1) == 10);
        TUT_ASSERT(str.indexOf('s', 10) == 10);
        TUT_ASSERT(str.indexOf(toFind, 11) < 0);
        
        TUT_ASSERT(str.indexOf(toFindInCase, 0, Qt::CaseSensitive) < 0);
        TUT_ASSERT(str.indexOf(toFindInCase, 0, Qt::CaseInsensitive) == 0);
        TUT_ASSERT(str.indexOf('S', 0, Qt::CaseSensitive) < 0);
        TUT_ASSERT(str.indexOf('S', 0, Qt::CaseInsensitive) == 0);

        TUT_ASSERT(str.lastIndexOf(toFind) == 10);
        TUT_ASSERT(str.lastIndexOf(toFind, 13) == 10);
        TUT_ASSERT(str.lastIndexOf(toFind, 5) == 0);
        TUT_ASSERT(str.lastIndexOf('x', 8) == -1);

        TUT_ASSERT(str.lastIndexOf(toFindInCase, -1, Qt::CaseSensitive) < 0);
        TUT_ASSERT(str.lastIndexOf(toFindInCase, -1, Qt::CaseInsensitive) == 10);
        TUT_ASSERT(str.lastIndexOf('S', -1, Qt::CaseSensitive) < 0);
        TUT_ASSERT(str.lastIndexOf('S', 5, Qt::CaseInsensitive) == 0);

        // contains
        TUT_ASSERT( str.contains( QChar('s') ) );
        TUT_ASSERT( str.contains(QChar('S'), Qt::CaseInsensitive) );
        TUT_ASSERT( !str.contains( QChar('x') ) );

        TUT_ASSERT( str.contains("s") );
        TUT_ASSERT( str.contains("S", Qt::CaseInsensitive) );
        TUT_ASSERT( !str.contains("x") );

        // count
        TUT_ASSERT(str.count(QChar('s')) == 2);
        TUT_ASSERT(str.count(QChar('S')) == 0);
        TUT_ASSERT(str.count(QChar('S'), Qt::CaseInsensitive) == 2);

        TUT_ASSERT(str.count(toFind) == 2);
        TUT_ASSERT(str.count(toFindInCase) == 0);
        TUT_ASSERT(str.count(toFindInCase, Qt::CaseInsensitive) == 2);

        // substr
        TUT_ASSERT(str.left(3) == toFind);
        TUT_ASSERT(str.left(str.size() + 4) == str);
        
        TUT_ASSERT(str.right(3) == "ion");
        TUT_ASSERT(str.right(str.size() + 4) == str);

        TUT_ASSERT(str.mid(10, 3) == toFind);
        TUT_ASSERT(str.mid(10, str.size() + 4) == "stion");
        
        // starts/ends
        TUT_ASSERT( str.startsWith( QChar('s') ) );
        TUT_ASSERT( str.startsWith(QChar('S'), Qt::CaseInsensitive) );
        TUT_ASSERT( !str.startsWith(QChar('S'), Qt::CaseSensitive) );

        TUT_ASSERT( str.startsWith(toFind) );
        TUT_ASSERT( str.startsWith(toFindInCase, Qt::CaseInsensitive) );
        TUT_ASSERT( !str.startsWith(toFindInCase, Qt::CaseSensitive) );

        TUT_ASSERT( str.endsWith( QChar('n') ) );
        TUT_ASSERT( str.endsWith(QChar('N'), Qt::CaseInsensitive) );
        TUT_ASSERT( !str.endsWith(QChar('N'), Qt::CaseSensitive) );

        TUT_ASSERT( str.endsWith( QString("ion") ) );
        TUT_ASSERT( str.endsWith(QString("iOn"), Qt::CaseInsensitive) );
        TUT_ASSERT( !str.endsWith(QString("ioN"), Qt::CaseSensitive) );
    }

    // ---------------------------------------------------------------

    /*
    QString trimmed() const;
    void truncate(int pos) ;
    void chop(int n);
    void clear();

    QString& remove(int i, int len);
    QString& remove(QChar c, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString& remove(const QString &s, Qt::CaseSensitivity cs = Qt::CaseSensitive);

    QString& replace(int i, int len, QChar after);
    QString& replace(int i, int len, const QChar *s, int slen);
    QString& replace(int i, int len, const QString &after);
    QString& replace(QChar before, QChar after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString& replace(QChar c, const QString &after, Qt::CaseSensitivity cs = Qt::CaseSensitive);
    QString& replace(const QString &before, const QString &after,
                     Qt::CaseSensitivity cs = Qt::CaseSensitive);
    */
    
    void TestRemoveReplace()
    {
        QString str(" ab c d  \t \t \t");
        str = str.trimmed();
        TUT_ASSERT(str == "ab c d");

        str.chop(2);
        TUT_ASSERT(str == "ab c");

        str.truncate(2);
        TUT_ASSERT(str == "ab");

        str.clear();
        TUT_ASSERT( str.isEmpty() );

        // remove
        TUT_ASSERT(QString("abcde").remove(1, 2) == "ade");
        TUT_ASSERT(QString("abcde").remove(1, 15) == "a");

        TUT_ASSERT(QString("ababAbAb").remove(QChar('a'), Qt::CaseSensitive) == "bbAbAb");
        TUT_ASSERT(QString("ababAbAb").remove(QChar('a'), Qt::CaseInsensitive) == "bbbb");

        TUT_ASSERT(QString("ababAbAb").remove(QString("ab"), Qt::CaseSensitive) == "AbAb");
        TUT_ASSERT( QString("ababAbAb").remove(QString("ab"), Qt::CaseInsensitive).isEmpty() );
        
        // replace
        TUT_ASSERT(QString("abcabc").replace( 1, 3, QChar('x') ) == "axbc");
        TUT_ASSERT(QString("abcabc").replace( 1, 15, QChar('x') ) == "ax");
        
        TUT_ASSERT(QString("ababcdef").replace( 0, 4, QString("cdef") ) == "cdefcdef");
        TUT_ASSERT(QString("ababcdef").replace( 4, -1, QString("ab") ) == "ababab");

        QChar after[2] = {'a', 'b'};
        TUT_ASSERT(QString("ababcdef").replace(4, -1, after, 2) == "ababab");
        TUT_ASSERT(QString("ababcdef").replace(2, 4, after, 2) == "ababef");

        TUT_ASSERT(QString("ababAb").replace(QChar('a'), QChar('x'), Qt::CaseSensitive) 
                   == "xbxbAb");
        TUT_ASSERT(QString("ababAb").replace(QChar('a'), QChar('x'), Qt::CaseInsensitive) 
                   == "xbxbxb");

        TUT_ASSERT(QString("ababAB").replace(QChar('b'), QString("bc"), Qt::CaseSensitive) 
                   == "abcabcAB");
        TUT_ASSERT(QString("ababAB").replace(QChar('B'), QString("bc"), Qt::CaseInsensitive) 
                   == "abcabcAbc");
        
        TUT_ASSERT(QString("abcdef abc").replace(QString("def"), QString(" abc"), 
                                                 Qt::CaseSensitive) == "abc abc abc");
        TUT_ASSERT(QString("abcdef abc").replace(QString("aBc"), QString("def"), 
                                                 Qt::CaseInsensitive) == "defdef def");
    }

    // ---------------------------------------------------------------

    /*
    QByteArray toAscii() const;
    static QString fromAscii(const char *str, int size = -1);

    QByteArray toLatin1() const;
    static QString fromLatin1(const char *str, int size = -1);

    static QString fromStdString(const std::string &s);
    std::string toStdString() const;

    static QString fromStdWString(const std::wstring &s);
    std::wstring toStdWString() const;

    int toInt(bool *ok=0, int base=10) const;
    uint toUInt(bool *ok=0, int base=10) const;
    
    QString& setNum(ushort num, int base=10);
    QString& setNum(int num, int base=10);
    static QString number(int num, int base=10);
    */
    
    void TestConvertion()
    {
        QString str("abc");
        wchar_t wchars[3] = {'a', 1589, 356};
        QString wstr( QString::fromStdWString( std::wstring(wchars, 3) ) );

        // ascii
        TUT_ASSERT(QString::fromAscii("abc") == str);
        TUT_ASSERT( str.toAscii() == QByteArray("abc") );
        TUT_ASSERT( wstr.toAscii() == QByteArray("a\0\0") );

        // latin1
        TUT_ASSERT( str.toLatin1() == QByteArray("abc") );
        TUT_ASSERT(QString::fromLatin1("abc") == str);
        TUT_ASSERT( wstr.toLatin1() == QByteArray("a\0\0") );

        // std
        std::string stdStr("abc");
        wchar_t wchars2[3] = {'a', 'b', 'c'};
        std::wstring stdWstr(wchars2, 3);
        
        TUT_ASSERT(QString::fromStdString(stdStr) == str);
        TUT_ASSERT(str.toStdString() == stdStr);

        TUT_ASSERT(QString::fromStdWString(stdWstr) == str);
        TUT_ASSERT(str.toStdWString() == stdWstr);

        // num
        QString strNum("159");

        // int
        int intNum = 159;
        TUT_ASSERT(strNum.toInt() == intNum);

        bool ok = true;
        TUT_ASSERT( (str.toInt(&ok) == 0) && (!ok) );

        // uint
        uint uintNum = 159;
        TUT_ASSERT(strNum.toUInt() == uintNum);

        ok = true;
        TUT_ASSERT( (str.toUInt(&ok) == 0) && (!ok) );

        // ushort
        ushort ushortNum = 159;
        TUT_ASSERT(QString().setNum(ushortNum) == strNum);
        TUT_ASSERT(QString().setNum(ushortNum) == strNum);

        TUT_ASSERT(QString::number(ushortNum) == strNum);
    }

    // ---------------------------------------------------------------

    /*
    QString& fill(QChar c, int size = -1); 

    QString& insert(int i, QChar c);
    QString& insert(int i, const QChar *uc, int len);
    QString& insert(int i, const QString &s);
    
    QString& append(QChar c);
    QString& append(const QString &s);
    QString& append(const char *s);
    QString& append(const QByteArray &s);

    QString& operator+=(const char *s);
    QString& operator+=(const QByteArray &s);
    QString& operator+=(char c);
    QString& operator+=(QChar c);
    QString& operator+=(QChar::SpecialCharacter c);
    QString& operator+=(const QString &s);

    void push_back(QChar c);
    void push_back(const QString &s);
    */
    
    void TestAddInsert()
    {
        // fill
        char ch = 'd';
        QChar qCh(ch);
        Check( QString().fill(qCh, 4), std::wstring(4, ch) );
        Check( QString("xxxxxxx").fill(qCh, 4), std::wstring(4, ch) );

        // append
        TUT_ASSERT( QString().append(qCh) == QString(qCh) );
        TUT_ASSERT( QString().append( QString(qCh) ) == QString(qCh) );
        TUT_ASSERT( QString("abc").append("def") == QString("abcdef") );
        TUT_ASSERT( QString().append( QByteArray("abc") ) == QString(QByteArray("abc")) );

        // insert
        TUT_ASSERT(QString("abc").insert(2, qCh) == "abdc");
        TUT_ASSERT(QString("abc").insert(3, qCh) == "abcd");

        TUT_ASSERT(QString("aef").insert(1, QString("bcd")) == "abcdef");
        TUT_ASSERT(QString("abc").insert(3, QString("def")) == "abcdef");

        QChar toIns[3] = {'d', 'e', 'f'};
        TUT_ASSERT(QString("abc").insert(3, toIns, 3) == "abcdef");

        // +=
        QString str("ab");
        TUT_ASSERT( (str += "cd") == "abcd" );
        TUT_ASSERT( ( str += QByteArray("e") ) == "abcde" );
        TUT_ASSERT( (str += 'f') == "abcdef" );
        TUT_ASSERT( ( str += QChar('0') ) == "abcdef0" );
        TUT_ASSERT( ( str += QString("123") ) == "abcdef0123" );
                         str.clear();
        QString str2( QChar(QChar::LineSeparator) );
        TUT_ASSERT( (str += QChar::LineSeparator) == str2 );
        
        // push_back
        str = "abc";
        str.push_back( QChar('d') );
        TUT_ASSERT(str == "abcd");
        str.push_back( QString("ef") );
        TUT_ASSERT(str == "abcdef");
    }

    // ---------------------------------------------------------------

    /*
    int compare(const QString &s) const;
    int compare(const QString &s, Qt::CaseSensitivity cs) const;
    static int compare(const QString &s1, const QString &s2);
    static int compare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs);
    
    bool operator==(const QString &s) const;
    bool operator<(const QString &s) const;
    bool operator>(const QString &s) const;
    bool operator!=(const QString &s) const;
    bool operator<=(const QString &s) const;
    bool operator>=(const QString &s) const;

    bool operator==(const char *s) const;
    bool operator!=(const char *s) const;
    bool operator<(const char *s) const;
    bool operator>(const char *s2) const;
    bool operator<=(const char *s2) const;
    bool operator>=(const char *s2) const;
    
    bool operator==(const QByteArray &s) const;
    bool operator!=(const QByteArray &s) const;
    bool operator<(const QByteArray &s) const;
    bool operator>(const QByteArray &s) const;
    bool operator<=(const QByteArray &s) const;
    bool operator>=(const QByteArray &s) const;
    */
    
    void TestCompare()
    {
        QString str1("abCde");
        
        TUT_ASSERT(QString().compare(str1) < 0);
        TUT_ASSERT(str1.compare(str1) == 0);
        TUT_ASSERT(str1.compare( QString("ab") ) > 0);

        TUT_ASSERT(QString::compare(QString(), str1) < 0);
        TUT_ASSERT(QString::compare(str1, str1) == 0);
        TUT_ASSERT(QString::compare(str1, QString("ab") ) > 0);

        
        TUT_ASSERT(str1.compare(QString("abcde"), Qt::CaseSensitive) < 0);
        TUT_ASSERT(str1.compare(QString("abcde"), Qt::CaseInsensitive) == 0);
        TUT_ASSERT(str1.compare(QString("abCDef"), Qt::CaseSensitive) > 0);
        TUT_ASSERT(str1.compare(QString("abcdEf"), Qt::CaseSensitive) < 0);
        TUT_ASSERT(str1.compare(QString("abcdef"), Qt::CaseInsensitive) < 0);

        TUT_ASSERT(QString::compare(str1, QString("Abcde"), Qt::CaseSensitive) > 0);
        TUT_ASSERT(QString::compare(str1, QString("abcde"), Qt::CaseInsensitive) == 0);
        TUT_ASSERT(QString::compare(str1, QString("abcdeF"), Qt::CaseSensitive) < 0);
        TUT_ASSERT(QString::compare(str1, QString("abcdef"), Qt::CaseInsensitive) < 0);

        CheckQStringCompareOperators( QString("ab"), QString("abc") );
        CheckQStringCompareOperators( QByteArray("ab"), QByteArray("abc") );
        // CheckQStringCompareOperators("ab", "abc");
        CheckQStringCompareOperators( QByteArray("ab").data(), QByteArray("abc").data() );
    }

} // namespace

// ---------------------------------------------------------------

namespace QtCoreTests
{
    void QStringTests()
    {
        TestConstructors();
        TestCopyAssignment();
        TestUtilities();
        TestDataAccess();
        TestFind();
        TestRemoveReplace();
        TestConvertion();
        TestAddInsert();
        TestCompare();
    }
    
} // namespace QtCoreTests

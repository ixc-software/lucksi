
#include "stdafx.h"

#include "qstring.h"
#include "Utils/ErrorsSubsystem.h"
// #include "qtextcodec.h"

namespace 
{
    bool MatchCh(QChar what, wchar_t source, Qt::CaseSensitivity cs)
    {
        switch (cs)
        {
        case Qt::CaseSensitive:
            return what.unicode() == source;

        case Qt::CaseInsensitive:
            return what.toUpper() == QChar(source).toUpper();

        default:
            ESS_ASSERT(0 && "Invalid Qt::CaseSensitivity");
        }

        return false;
    }

    // -----------------------------------------------------

    bool MatchStr(const QString &what, const std::wstring &source, Qt::CaseSensitivity cs)
    {
        ESS_ASSERT(what.size() >= source.size());
        if ( what.size() > source.size() ) return false;
        
        for (int i = 0; i < what.size(); ++i)
        {
            if ( !MatchCh(what.at(i), source.at(i), cs) ) return false;
        }

        return true;
    }

    // -----------------------------------------------------

    void FindAllCh(QChar what, const std::wstring &where, int from, Qt::CaseSensitivity cs, 
                   std::vector<int> &outIndexes)
    {
        outIndexes.clear();

        for (size_t i = from; i < where.size(); ++i)
        {
            if ( MatchCh(what, where.at(i), cs) ) outIndexes.push_back(i);
        }
    }

    // -----------------------------------------------------

    // what = "zz", where = "zzzz" --> outIndexes.size() == 2
    void FindAllStr(const QString &what, const std::wstring &where, int from, 
                    Qt::CaseSensitivity cs, std::vector<int> &outIndexes)
    {
        outIndexes.clear();

        int whatSize = what.size();
        size_t i = from;
        while (i < where.size())
        {
            if ( MatchStr(what, where.substr(i, whatSize), cs) )
            {
                outIndexes.push_back(i);
                i += whatSize - 1;
            }
            ++i;
        }
    }

    // ------------------------------------------------------

    template<class NumType>
    std::string NumToWstring(NumType num, int base)
    {
        std::stringstream ss;
        ss << num;
        
        return ss.str();
    }

    // ------------------------------------------------------

    bool IsSpace(const std::wstring &data, int index)
    {
        return QChar( data.at(index) ).isSpace();
    }

    // -

    template<class TNum>
    TNum StringToNum(const QString &data, bool *ok, int base)
    {
        std::stringstream ss;
        ss << data.toAscii().data();
        TNum res = 0;
        bool fail = (ss >> res).fail();
        if (ok != 0) *ok = !fail; 
       
        return (fail)? 0: res;
    }

} // namespace 

// ------------------------------------------------------------------------

// QTextCodec *QString::codecForCStrings;

// ------------------------------------------------------------------------

QString::QString(const QChar *unicode, int size)
{
    int i = 0;
    while (i < size)
    {
        m_data += unicode[i].unicode();
        ++i;
    }
}

// ------------------------------------------------------------------------

QString::QString(const char *ch) 
{
    std::string data(ch);
    
    m_data = QString::fromAscii( data.c_str(), data.size() ).toStdWString(); 
}

// ------------------------------------------------------------------------

QString& QString::operator=(QChar c)
{
    m_data.clear();
    m_data = c.unicode();
    
    return *this;
}

// ------------------------------------------------------------------------
    
QString& QString::operator=(const QString &other)
{
    m_data = other.m_data;
    
    return *this;
}

// ------------------------------------------------------------------------

int QString::indexOf(QChar c, int from, Qt::CaseSensitivity cs) const
{
    ESS_ASSERT( from < size() );
    std::vector<int> indx;
    FindAllCh(c, m_data, from, cs, indx);
    
    return (indx.size() == 0)? -1 : indx.at(0); // return first index
}

// ------------------------------------------------------------------------ 

int QString::indexOf(const QString &s, int from, Qt::CaseSensitivity cs) const
{
    ESS_ASSERT( from < size() );
    std::vector<int> indx;
    FindAllStr(s, m_data, from, cs, indx);

    return (indx.size() == 0)? -1 : indx.at(0); // return first index
}

// ------------------------------------------------------------------------

int QString::lastIndexOf(QChar c, int from, Qt::CaseSensitivity cs) const
{
    ESS_ASSERT( (from >= -1) && ( from < size() ) ); 

    std::vector<int> indx;
    FindAllCh(c, m_data.substr(0, from), 0, cs, indx);

    return (indx.size() == 0)? -1 : indx.at(indx.size() - 1);
}

// ------------------------------------------------------------------------

int QString::lastIndexOf(const QString &s, int from, Qt::CaseSensitivity cs) const
{
    ESS_ASSERT( (from >= -1)  && (from < size()) ); 

    std::vector<int> indx;
    FindAllStr(s, m_data.substr(0, from), 0, cs, indx);

    return (indx.size() == 0)? -1 : indx.at(indx.size() - 1);
}

// ------------------------------------------------------------------------

QString& QString::fill(QChar c, int size) 
{ 
    m_data.clear();
    for (int i = 0; i < size; ++i)
    {
        m_data += c.unicode();
    }
    
    return *this;
}

// ------------------------------------------------------------------------

void QString::truncate(int pos) 
{
    if ( pos > size() ) pos = std::wstring::npos;
    m_data = m_data.substr(0, pos);
}

// ------------------------------------------------------------------------

void QString::chop(int n)
{
    if ( n >= size() ) clear(); 
    else m_data = m_data.erase(m_data.size() - n);
}

// ------------------------------------------------------------------------

QBool QString::contains(QChar c, Qt::CaseSensitivity cs) const
{
    return QBool(indexOf(c, 0, cs) >= 0);
}

// ------------------------------------------------------------------------

QBool QString::contains(const QString &s, Qt::CaseSensitivity cs) const
{
    return QBool(indexOf(s, 0, cs) >= 0);
}

// ------------------------------------------------------------------------

int QString::count(QChar c, Qt::CaseSensitivity cs) const
{
    std::vector<int> indexes;
    FindAllCh(c, m_data, 0, cs, indexes);

    return indexes.size();
}

// ------------------------------------------------------------------------

int QString::count(const QString &s, Qt::CaseSensitivity cs) const
{
    std::vector<int> indexes;
    FindAllStr(s, m_data, 0, cs, indexes);

    return indexes.size();
}

// ------------------------------------------------------------------------

QString QString::left(int n) const 
{ 
    return QString( QString::fromStdWString( m_data.substr(0, n) ) );
}

// ------------------------------------------------------------------------

QString QString::right(int n) const 
{ 
    if (n >= size()) return QString(*this);

    return QString( QString::fromStdWString( m_data.substr( (size() - n), n ) ) );
}

// ------------------------------------------------------------------------

QString QString::mid(int position, int n) const 
{ 
    ESS_ASSERT( position < size() );

    return QString( QString::fromStdWString( m_data.substr(position, n) ) );
}

// ------------------------------------------------------------------------

bool QString::startsWith(const QString &s, Qt::CaseSensitivity cs) const
{
    return (indexOf(s, 0, cs) == 0);
}

// ------------------------------------------------------------------------

bool QString::startsWith(const QChar &c, Qt::CaseSensitivity cs) const
{
    return (indexOf(c, 0, cs) == 0);
}

// ------------------------------------------------------------------------

bool QString::endsWith(const QString &s, Qt::CaseSensitivity cs) const
{
    return ( lastIndexOf(s, -1, cs) == ( size() - s.size() ) );
}

// ------------------------------------------------------------------------

bool QString::endsWith(const QChar &c, Qt::CaseSensitivity cs) const
{
    return ( lastIndexOf(c, -1, cs) == (size() - 1) );
}

// ------------------------------------------------------------------------

QString QString::toLower() const
{
    QString res;
    for(int i = 0; i < size(); ++i)
    {
        res += QChar(m_data.at(i)).toLower();
    }
    
    return res;
}

// ------------------------------------------------------------------------

QString QString::toUpper() const
{
    QString res;
    for (int i = 0; i < size(); ++i)
    {
        res += QChar(m_data.at(i)).toUpper();
    }
    
    return res;
}

// ------------------------------------------------------------------------

QByteArray QString::toAscii() const
{
    return toLatin1();
}

// ------------------------------------------------------------------------
/*
const char QString::toAscii() const 
{ 
    ESS_ASSERT(size() == 1);
    return QChar( m_data.at(0) ).toAscii();
}
*/
// ------------------------------------------------------------------------

QString QString::fromAscii(const char *str, int size)
{
    return fromLatin1(str, size);
}

// ------------------------------------------------------------------------

QByteArray QString::toLatin1() const
{
    QByteArray res;
    res.reserve(size());

    for (int i = 0; i < size(); ++i)
    {
        QChar ch( m_data.at(i) );
        res.push_back( ch.toLatin1() ); 
    }

    return res;
}

// ------------------------------------------------------------------------

QString QString::fromLatin1(const char *str, int size)
{
    QString res;
    std::string data(str);
    for (int i = 0; i < data.size(); ++i)
    {
        res += QChar::fromLatin1( data.at(i) );
    }

    return res;
}

// ------------------------------------------------------------------------

QString QString::trimmed() const
{
    int begin = 0;
    int end = size() - 1;
    while ( IsSpace(m_data, begin) ) ++begin;
    while ( IsSpace(m_data, end) ) -- end;
    ++end;
    
    return mid( begin, (end - begin) );
}

// ------------------------------------------------------------------------

QString& QString::insert(int i, QChar c) 
{ 
    if (i >= m_data.size()) m_data.resize(i);
    m_data.insert(i, 1, c.unicode()); 
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::insert(int i, const QChar *uc, int len)
{
    return insert( i, QString(uc, len) );
}

// ------------------------------------------------------------------------

QString& QString::insert(int i, const QString &s) 
{ 
    if (i >= m_data.size()) m_data.resize(i);
    m_data.insert( i, s.toStdWString() );
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::append(QChar c)
{
    m_data += c.unicode();
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::append(const QString &s) 
{
    m_data += s.toStdWString();
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::append(const char *s) 
{ 
    return append(QString::fromAscii(s)); 
}

// ------------------------------------------------------------------------

QString& QString::remove(int i, int len)
{
    ESS_ASSERT( i < size() );
    m_data.erase(i, len);
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::remove(QChar c, Qt::CaseSensitivity cs)
{
    int i = 0;
    while ( i < size() )
    {
        if ( MatchCh(c, m_data.at(i), cs) )
        {
            m_data.erase(i, 1);
            --i;
        }
        ++i;
    }
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::remove(const QString &s, Qt::CaseSensitivity cs)
{
    int i = 0;
    while ( i < size() )
    {
        if ( MatchStr(s, m_data.substr( i, s.size() ), cs) )
        {
            m_data.erase( i, s.size() );
            --i;
        }
        ++i;
    }
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::replace(int i, int len, QChar after)
{
    ESS_ASSERT( i < size() );
    m_data.replace(i, len, 1, after.unicode());
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::replace(int i, int len, const QChar *s, int slen)
{
    return replace( i, len, QString(s, slen) );
}

// ------------------------------------------------------------------------

QString& QString::replace(int i, int len, const QString &after)
{
    ESS_ASSERT( i < size() );
    m_data.replace( i, len, after.toStdWString() );
    
    return *this;
}

// ------------------------------------------------------------------------

QString& QString::replace(QChar before, QChar after, Qt::CaseSensitivity cs)
{
    return replace(QString(before), QString(after), cs); 
}

// ------------------------------------------------------------------------

QString& QString::replace(QChar c, const QString &after, Qt::CaseSensitivity cs)
{
    return replace(QString(c), after, cs);
}

// ------------------------------------------------------------------------

QString& QString::replace(const QString &before, const QString &after, Qt::CaseSensitivity cs)
{
    int i = 0;
    while ( i < size() )
    {
        if ( MatchStr(before, m_data.substr( i, before.size() ), cs) )
        {
            m_data.replace( i, before.size(), after.toStdWString() );
            i += after.size() - 1;
        }
        ++i;
    }
    
    return *this;
}

// ------------------------------------------------------------------------

int QString::compare(const QString &s) const
{
    return compare(s, Qt::CaseSensitive);
}

//------------------------------------------------------------------------

int QString::compare(const QString &s, Qt::CaseSensitivity cs) const
{
    int res = m_data.compare( s.toStdWString() );
    if (cs == Qt::CaseInsensitive)
    {
        // "AbCd" == "abcd"
        if ( (res != 0) && ( size() == s.size() ) ) res = 0;
    }
    
    return res;
}

// ------------------------------------------------------------------------

int QString::compare(const QString &s1, const QString &s2) 
{ 
    return s1.compare(s2); 
}

// ------------------------------------------------------------------------

int QString::compare(const QString &s1, const QString &s2, Qt::CaseSensitivity cs) 
{ 
    return s1.compare(s2, cs); 
}

// ------------------------------------------------------------------------

int QString::toInt(bool *ok, int base) const
{
    /*
    std::stringstream ss;
    ss << toAscii().data();
    int res = 0;
    bool fail = (ss >> res).fail();
    if (ok != 0) *ok = !fail; 
   
    return (fail)? 0: res;
    */

    int res = StringToNum<int>(*this, ok, base);
    
    return res;
}

// ------------------------------------------------------------------------

uint QString::toUInt(bool *ok, int base) const
{
    uint res = StringToNum<uint>(*this, ok, base);
    
    return res;
}

// ------------------------------------------------------------------------

QString& QString::setNum(ushort num, int base)
{
    std::string strNum = NumToWstring(num, base);
    
    return ( *this = QString::fromAscii( strNum.c_str(), strNum.size() ) );
}

// ------------------------------------------------------------------------

QString& QString::setNum(int num, int base)
{
    std::string strNum = NumToWstring(num, base);
    
    return ( *this = QString::fromAscii( strNum.c_str(), strNum.size() ) );
}

// ------------------------------------------------------------------------

QString QString::number(int num, int base)
{
    return QString().setNum(num, base);
}

// ------------------------------------------------------------------------

QString QString::fromStdString(const std::string &s)
{
    return QString( s.c_str() );
}

// ------------------------------------------------------------------------

std::string QString::toStdString() const
{
    return std::string( toAscii().data() );
}

// ------------------------------------------------------------------------

QString QString::fromStdWString(const std::wstring &s)
{
    QString res;
    for (int i = 0; i < s.size(); ++i)
    {
        res += QChar( s.at(i) );
    }
    
    return res;
}

// ------------------------------------------------------------------------

wchar_t& QString::operator[](int i) 
{ 
    ESS_ASSERT( i < size() );
    return m_data[i]; 
} 

// ------------------------------------------------------------------------

wchar_t& QString::operator[](uint i) 
{ 
    ESS_ASSERT( i < size() );
    return m_data[i]; 
}

// ------------------------------------------------------------------------


#include "stdafx.h"

#include "qbytearray.h"
#include "qchar.h"


namespace 
{
    bool IsSpace(const std::string &data, int index)
    {
        return QChar( data.at(index) ).isSpace();
    }
    
} // namespace 

// -----------------------------------------------------------------

QByteArray& QByteArray::operator=(const QByteArray &other)
{
    m_data = other.m_data;
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::operator=(const char *str)
{
    m_data = str;
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::fill(char c, int size)
{
    return ( *this = QByteArray(size, c) );
}

// -----------------------------------------------------------------

char& QByteArray::operator[](int i)
{
    ESS_ASSERT( i < size() );
    return m_data[i];
}

// -----------------------------------------------------------------

char& QByteArray::operator[](uint i)
{
    ESS_ASSERT( i < size() );
    return m_data[i];
}

// -----------------------------------------------------------------

int QByteArray::indexOf(char c, int from) const
{
    ESS_ASSERT( from < size() );
    
    return m_data.find(c, from);
}

// -----------------------------------------------------------------

int QByteArray::indexOf(const char *c, int from) const
{
    ESS_ASSERT( from < size() );
    
    return m_data.find(c, from); 
}

//-----------------------------------------------------------------

int QByteArray::indexOf(const QByteArray &a, int from) const
{
    return indexOf(a.data(), from);
}

// -----------------------------------------------------------------

int QByteArray::lastIndexOf(char c, int from) const
{
    ESS_ASSERT( (from >= -1) && (from < size()) ); 

    return m_data.rfind(c, from);
}

// -----------------------------------------------------------------

int QByteArray::lastIndexOf(const char *c, int from) const
{
    ESS_ASSERT( (from >= -1) && (from < size()) ); 

    std::string toFind(c);

    return m_data.rfind(toFind, from);
}

// -----------------------------------------------------------------

int QByteArray::lastIndexOf(const QByteArray &a, int from) const
{
    return lastIndexOf(a.data(), from);
}

// -----------------------------------------------------------------
    
QByteArray QByteArray::left(int len) const
{
    return QByteArray( m_data.substr(0, len).c_str() );
}

// -----------------------------------------------------------------

QByteArray QByteArray::right(int len) const
{
    if (len >= size()) return QByteArray(*this);
    
    return QByteArray( m_data.substr( (size() - len), len ).c_str() );
}

// -----------------------------------------------------------------

QByteArray QByteArray::mid(int index, int len) const
{
    ESS_ASSERT( index < size() );
    
    return QByteArray( m_data.substr(index, len).c_str() );
}

// -----------------------------------------------------------------

void QByteArray::truncate(int pos)
{
    if ( pos > size() ) pos = std::string::npos;
    m_data = m_data.substr(0, pos);
}

// -----------------------------------------------------------------

void QByteArray::chop(int n)
{
    if ( n >= size() ) clear();
    else m_data = m_data.erase(m_data.size() - n);
}

// -----------------------------------------------------------------

QByteArray QByteArray::trimmed() const
{
    int begin = 0;
    int end = size() - 1;
    while ( IsSpace(m_data, begin) ) ++begin;
    
    while ( IsSpace(m_data, end) ) -- end;
    ++end;
    
    return mid( begin, (end - begin) );
}

// -----------------------------------------------------------------

QByteArray& QByteArray::append(char c) 
{ 
    m_data.append(1, c); 
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::append(const char *s) 
{ 
    m_data.append(s); 
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::append(const QByteArray &a) 
{ 
    m_data.append( a.data() );
     
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::insert(int i, char c) 
{ 
    if (i >= m_data.size()) m_data.resize(i);
    m_data.insert(i, 1, c); 
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::insert(int i, const char *s)
{
    if (i >= m_data.size()) m_data.resize(i);
    m_data.insert(i, s); 
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::insert(int i, const QByteArray &a)
{
    return insert( i, a.data() );
}

// -----------------------------------------------------------------

QByteArray& QByteArray::remove(int index, int len)
{
    ESS_ASSERT( index < size() );
    m_data.erase(index, len);
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::replace(int index, int len, const char *s)
{
    ESS_ASSERT(index < size());
    m_data.replace(index, len, s);
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::replace(int index, int len, const QByteArray &s)
{
    return replace( index, len, s.data() );
}

// -----------------------------------------------------------------

QByteArray& QByteArray::operator+=(char c)
{
    m_data += c;
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::operator+=(const char *s)
{
    m_data += s;
    
    return *this;
}

// -----------------------------------------------------------------

QByteArray& QByteArray::operator+=(const QByteArray &a)
{
    return operator+=( a.data() );
}

// -----------------------------------------------------------------

void QByteArray::push_back(const char *c)
{
    std::string tmp(c);
    for (int i = 0; i < tmp.size(); ++i)
    {
        m_data.push_back( tmp.at(i) );
    }
}

// -----------------------------------------------------------------


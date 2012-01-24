#include "stdafx.h"
#include "IntSet.h"

#include "IntToString.h"
#include "StringUtils.h"
#include "StringParser.h"
#include "RangeList.h"

namespace Utils
{
    IntSet::IntSet( int capacity, const std::string &inStr, const std::string &separator /*= ","*/, const std::string &seg /*= ".."*/ )
    {
        if (!Parse(capacity, inStr, separator, seg)) ESS_THROW(ParseException);
    }

    // ------------------------------------------------------------------------------------

    IntSet::IntSet( const std::string &inStr, const std::string &separator /*= ","*/, const std::string &seg /*= ".."*/ )
    {
        if (!Parse(NoLimit, inStr, separator, seg)) ESS_THROW(ParseException);
    }

    // ------------------------------------------------------------------------------------

    IntSet::IntSet( int capacity, const std::vector<int> &par )
    {
        m_capacity = capacity;
        for(std::vector<int>::const_iterator i = par.begin(); i != par.end(); ++i)
        {
            Set(*i);
        }
    }

    // ------------------------------------------------------------------------------------

    IntSet::IntSet( int capacity /*= NoLimit*/ ) : m_capacity(capacity)
    {

    }

    // ------------------------------------------------------------------------------------

    bool IntSet::Parse(int capacity, const std::string &inStr, 
        const std::string &separator, 
        const std::string &seg)
    {
        m_capacity = capacity;
        return IntContParse(inStr, m_impl, capacity, separator, seg);
    };

    // ------------------------------------------------------------------------------------

    bool IntSet::Parse( const std::string &inStr, const std::string &separator /*= ","*/, const std::string &seg /*= ".."*/ )
    {
        return Parse(m_capacity, inStr, separator, seg);
    }

    // ----------------------------------------------------------------------

    IntSet IntSet::LowSlice(int count) const
    {
        if(count >= m_impl.size()) return *this;

        IntSet res(Capacity());        

        for(ImplType::const_iterator i = m_impl.begin(); i != m_impl.end() && count != 0; ++i)
        {
            --count;
            res.Set(*i);
        }
        return res;			
    }

    // ----------------------------------------------------------------------    

    IntSet IntSet::HighSlice(int count) const
    {
        if(count >= m_impl.size()) return *this;

        IntSet res(Capacity());
        for(ImplType::const_reverse_iterator i = m_impl.rbegin(); i != m_impl.rend() && count != 0; ++i)
        {
            --count;
            res.Set(*i);
        }
        return res;			
    }
    
    // ----------------------------------------------------------------------

    std::string IntSet::ToString(const std::string &separator, const std::string &seg) const
    {
        return IntContToString(m_impl, separator, seg);
    }

    // ------------------------------------------------------------------------------------

    void IntSet::Include( const IntSet &set )
    {
        m_impl.insert(set.m_impl.begin(), set.m_impl.end());
    }

    // ------------------------------------------------------------------------------------

    void IntSet::Exclude( const IntSet &set )
    {
        for(ImplType::const_iterator i = set.m_impl.begin(); i != set.m_impl.end(); ++i)
        {
            ImplType::iterator del = m_impl.find(*i);
            if(del != m_impl.end()) m_impl.erase(del);
        }
    }

    // ------------------------------------------------------------------------------------

    Utils::IntSet IntSet::Difference( const IntSet &set ) const
    {
        IntSet diff;
        for(ImplType::const_iterator i = m_impl.begin(); i != m_impl.end(); ++i)
        {
            if(set.m_impl.find(*i) == set.m_impl.end()) diff.m_impl.insert(*i);
        }
        return diff;
    }

    // ------------------------------------------------------------------------------------

    bool IntSet::IsContain( const IntSet &set ) const
    {
        for(ImplType::const_iterator i = set.m_impl.begin(); i != set.m_impl.end(); ++i)
        {
            if(m_impl.find(*i) == m_impl.end()) return false;
        }
        return true;
    }

    // ------------------------------------------------------------------------------------

    bool IntSet::operator==( const IntSet &set ) const
    {
        return m_impl == set.m_impl;
    }

    // ------------------------------------------------------------------------------------

    bool IntSet::operator!=( const IntSet &set ) const
    {
        return !(*this == set);
    }

    // ------------------------------------------------------------------------------------

    int IntSet::Size() const
    {
        return m_impl.size();
        //return capacity;
    }

    // ------------------------------------------------------------------------------------

    bool IntSet::IsExist( int i ) const
    {
        ESS_ASSERT(ValidIndex(i));
        return m_impl.find(i) != m_impl.end();
    }

    // ------------------------------------------------------------------------------------

    void IntSet::Set( int i )
    {
        ESS_ASSERT(ValidIndex(i));
        m_impl.insert(i);
    }

    // ------------------------------------------------------------------------------------

    void IntSet::Clear( int i )
    {
        ESS_ASSERT(ValidIndex(i));
        m_impl.erase(i);
    }

    // ------------------------------------------------------------------------------------

    int IntSet::MinValue() const
    {
        return *m_impl.begin();
    }

    // ------------------------------------------------------------------------------------

    int IntSet::MaxValue() const
    {
        return *m_impl.rbegin();
    }

    // ------------------------------------------------------------------------------------

    void IntSet::ConvertToVector( std::vector<int> &out ) const
    {
        for(ImplType::const_iterator i = m_impl.begin(); i != m_impl.end(); ++i)
        {
            out.push_back(*i);
        }
    }

    // ------------------------------------------------------------------------------------

    bool IntSet::ValidIndex( int i ) const
    {
        if (Capacity() == NoLimit) return true;

        return i >= 0  && (i < Capacity() || Capacity()  == 0);
    }

    // ------------------------------------------------------------------------------------

    bool IntSet::ConvertToInt( const std::string &inStr, int &result )
    {
        std::string str = TrimBlanks(inStr);
        return StringToInt(str, result);
    }

    // ------------------------------------------------------------------------------------

    Utils::IntSet32 IntSet32::Difference( const IntSet32& set ) const
    {
        return IntSet32( Utils::IntSet::Difference(set) );
    }

    // ------------------------------------------------------------------------------------

    Utils::IntSet32 IntSet32::LowSlice( int count ) const
    {
        return IntSet32( IntSet::LowSlice(count) );
    }

    // ------------------------------------------------------------------------------------

    Utils::IntSet32 IntSet32::HighSlice( int count ) const
    {
        return IntSet32( IntSet::HighSlice(count) );
    }

    // ------------------------------------------------------------------------------------

    IntSet32::IntSet32( const std::string& inStr ) : IntSet(Capacity(), inStr)
    {
    }

    // ------------------------------------------------------------------------------------

    IntSet32::IntSet32( const IntSet& set ) : IntSet(set)
    {
    }

    // ------------------------------------------------------------------------------------

    IntSet32::IntSet32() : IntSet(Capacity())
    {
    }
} // namespace Utils


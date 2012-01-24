#ifndef __COMPARABLET__
#define __COMPARABLET__

namespace Utils
{
	
    template<class T>
    class ComparableT
    {

        int Compare(const T &other) const
        {
            const T& base = static_cast<const T&>(*this); 
            return base.Compare(other);
        }

    protected:

        ComparableT()
        {
            const static bool CBase = boost::is_base_of<ComparableT<T>, T>::value;
            BOOST_STATIC_ASSERT(CBase);
        }

    public:

        bool operator==(const T &other) const { return (Compare(other) == 0); }
        bool operator!=(const T &other) const { return (Compare(other) != 0); }
        bool operator>=(const T &other) const { return (Compare(other) >= 0); }
        bool operator<=(const T &other) const { return (Compare(other) <= 0); }
        bool operator>(const T &other)  const { return (Compare(other) > 0); }
        bool operator<(const T &other)  const { return (Compare(other) < 0); }
    };

    // ----------------------------------------------------------------

    template<class T1, class T2>  
    int CompareToIntWeak(const T1 &v1, const T2 &v2)
    {
        if (v1 > v2) return 1;
        if (v1 < v2) return -1;
        return 0;
    }

    template<class T>  
    int CompareToInt(const T &v1, const T &v2)
    {
        return CompareToIntWeak(v1, v2);
    }

    // ----------------------------------------------------------------

    /*
    template<class T>
    int DefaultContElementCompare(const T &v1, const T &v2)
    {
        return CompareToInt(v1, v2);
    } */

    template<class TCont1, class TCont2, class TCompareFn>
    int CompareContToInt(const TCont1 &c1, const TCont2 &c2, TCompareFn cmpFn)
    {
        int sizeCmp = CompareToInt(c1.size(), c2.size());
        if (sizeCmp != 0) return sizeCmp;

        typename TCont1::const_iterator i1 = c1.begin();
        typename TCont2::const_iterator i2 = c2.begin();

        while(i1 != c1.end())
        {
            ESS_ASSERT(i2 != c2.end());

            int cmp = cmpFn(*i1, *i2);
            if (cmp != 0) return cmp;

            ++i1;
            ++i2;
        }

        return 0;
    }

    template<class TCont>
    int CompareContToInt(const TCont &c1, const TCont &c2)
    {
        return CompareContToInt(c1, c2, CompareToInt<typename TCont::value_type>);
    }

    /*

    Example:

    class IntClass : public ComparableT<IntClass>
    {
        int m_value;

        int Compare(const IntClass &other) const
        {
            return CompareToInt(m_value, other.m_value);
        }
    };
    */
	
	
} // namespace Utils

#endif


#ifndef _BIT_SET_IPSIUS_H_
#define _BIT_SET_IPSIUS_H_

#include "stdafx.h"
#include "RangeList.h"
#include "IntToString.h"
#include "ErrorsSubsystem.h"

namespace Utils
{    
    class IntSet 
    {
        typedef std::set<int> ImplType;
		
        enum {NoLimit = -1};
    public:

        ESS_TYPEDEF(ParseException);
        
        IntSet(int capacity = NoLimit);         
        IntSet(
            int capacity,
            const std::string &inStr,
            const std::string &separator = ",", const std::string &seg = ".."
            );

        IntSet(
            const std::string &inStr, 
            const std::string &separator = ",", 
            const std::string &seg = "..");

        IntSet(int capacity, const std::vector<int> &par); 

        int Capacity() const { return m_capacity;  }

        IntSet LowSlice(int count) const;
        IntSet HighSlice(int count) const;

        bool Parse(int capacity, const std::string &inStr, 
            const std::string &separator = ",", 
            const std::string &seg = "..");

        bool Parse(const std::string &inStr, 
            const std::string &separator = ",", 
            const std::string &seg = "..");

        std::string ToString(const std::string &separator = ", ", 
            const std::string &seg = "..") const;
        
        void Include(const IntSet &set);
        void Exclude(const IntSet &set);
        IntSet Difference(const IntSet &set) const;       
        bool IsContain(const IntSet &set) const;        
        bool operator == (const IntSet &set) const;
        bool operator != (const IntSet &set) const;
        int Size() const;
        int Count() const { return m_impl.size();  }
        bool IsEmpty() const {  return m_impl.size() == 0; }
        bool IsExist(int i) const;
        void Set(int i);
        void Clear() {  m_impl.clear(); }
        void Clear(int i);
        int MinValue() const;
        int MaxValue() const;
        void ConvertToVector(std::vector<int> &out) const;
		
		typedef ImplType::const_iterator ConstIterator;		
		typedef ImplType::iterator Iterator;				
		ConstIterator Begin() const	{	return m_impl.begin();	}
		ConstIterator End() const	{	return m_impl.end();	}

		Iterator Begin(){	return m_impl.begin();	}
		Iterator End()	{	return m_impl.end();	}

		void Clear(Iterator &i) {	m_impl.erase(i); }

    private:
        bool ValidIndex(int i) const;
        static bool ConvertToInt(const std::string &inStr, int &result);
    
    private:
        ImplType m_impl;
        int m_capacity;
    };

    // -----------------------------------------------------

    // IntSet with capacity = 32
    class IntSet32 : public IntSet
    {
        IntSet32(const IntSet& set);
    public:

        IntSet32();        
        IntSet32 Difference(const IntSet32& set) const;
        IntSet32 LowSlice(int count) const;
        IntSet32 HighSlice(int count) const;
        IntSet32(const std::string& inStr);
        static int Capacity() { return 32; }
    };

    // -----------------------------------------------------

    template<class TCont>
    std::string IntContToString(const TCont &v,
        const std::string &separator = ", ", 
        const std::string &seg = "..") 
    {
        if ( v.empty() ) return "";

        std::string result;
        int segmentCount = 0;
        typename TCont::value_type  prev = *v.begin();
        result += IntToString(prev);

        for(typename TCont::const_iterator curr = ++v.begin();
            curr != v.end();
            ++curr)
        {

            if( *curr == prev + 1 ) 
            {
                // сегмент продолжается
                prev = *curr;
                ++segmentCount;
                continue;
            } 

            if(segmentCount > 1) 
            {
                result += seg;
                segmentCount = 0;
                result += IntToString(prev);
            }

            prev = *curr;
            result += separator;
            result += IntToString(*curr);
        }

        if(segmentCount != 0)
        {
            result += (segmentCount == 1) ? separator : seg;
            result += IntToString(prev);
        }

        return result;
    }    

    // -----------------------------------------------------

    struct IntContParseUtils
    {

        static void Add(std::vector<int> &v, int value)
        {
            v.push_back(value);
        }

        static void Add(std::set<int> &v, int value)
        {
            v.insert(value);
        }

    };


    template<class TCont>
    bool IntContParse(const std::string &inStr, TCont &result,
        int maxCapacity = -1,
        const std::string &separator = ", ", 
        const std::string &seg = "..")
    {
        result.clear();

        std::string err;
        boost::scoped_ptr<RangeList> ranges( ( RangeList::Create(inStr, err, separator, seg) ) );
        if (!ranges) return false;

        for (int i = 0; i < ranges->Size(); ++i)
        {   
            const ValueRange &r = ranges->At(i);
            int val = r.From();

            while(true)
            {
                // verify
                if (maxCapacity > 0)
                {
                    if ((val < 0) || (val >= maxCapacity)) return false;
                }

                // add
                IntContParseUtils::Add(result, val);

                // next
                ++val;

                if (val > r.To()) break;
            }
        }

        return true;
    };



}  // namespace Utils

#endif

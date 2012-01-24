#include "stdafx.h"
#include "RangeList.h"
#include "StringParser.h"
#include "StringUtils.h"
#include "ManagedList.h"
#include "Random.h"

namespace 
{
    using Platform::ddword;
    using namespace Utils;
    const ddword CMaxDDword = ~ddword(0);

    bool StringToDDword(const std::string& s, ddword& result)
    {
        Utils::TrimBlanks(s);

        std::istringstream ss;
        ss.str(s);
        if (s.substr(0, 2) == "0x")
            ss >> std::hex >> result;
        else                    
            ss >> result;

        if (ss.fail()) return false;

        char dummy;
        ss >> dummy;

        if (ss.fail()) 
            return true;
        else
            return false;
    }

    // ------------------------------------------------------------------------------------

    void TestParseRange(const std::string& s, ddword expFrom, ddword expTo, bool errorExpected = false)
    {        
        try
        {
            ValueRange range(s);
            TUT_ASSERT(!errorExpected);     
            for (ddword i = expFrom; i <= expTo; ++i)
            {
                TUT_ASSERT(range.ExistValue(i));
                if (i == CMaxDDword) break;
            }
        }
        catch(Utils::RangeParserError& e)
        {
            TUT_ASSERT(errorExpected);     
        }
    }

    // ------------------------------------------------------------------------------------

    void TestParseRangeList(const std::string& s, int expectedSize, bool errorExpected = false)
    {
        boost::shared_ptr<RangeList> result;
        std::string err;
        result.reset( RangeList::Create(s, err) );
        if (!result) 
        {
            TUT_ASSERT(errorExpected && "ParseFailed");
        }
        else
        {
            TUT_ASSERT(!errorExpected && "ParseErrorExpected");                    
            // разбит на ожидаемое количество диапазонов
            TUT_ASSERT(result->Size() == expectedSize);            
            // Rangelist-->string-->RangeList
            TUT_ASSERT(RangeList(result->ToString()).Size() == result->Size());
        }
        return;
    }  

    // ------------------------------------------------------------------------------------

    std::string ValToString(ddword val)
    {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }        
    
} // namespace 

namespace Utils
{
    ValueRange::ValueRange( const std::string& inStr, const std::string& sep /*= "-"*/ )        
    {
        if (inStr.empty()) ESS_THROW_MSG(RangeParserError, "Value not present");        
        const std::string s = Utils::TrimBlanks(inStr);

        Utils::ManagedList<std::string> sl;        
        Utils::StringParser(s, sep, sl);
        ESS_ASSERT(!sl.IsEmpty());

        if (sl.Size() > 2) 
            ESS_THROW_MSG(RangeParserError, "Wrong value: " + s);

        // read m_fromNum
        if (!StringToDDword(*sl[0], m_fromVal)) 
            ESS_THROW_MSG(RangeParserError, "Can`t convert val: " + *sl[0]);

        //read m_toNum
        if (sl.Size() == 2) 
        {
            // x-
            if (sl[1]->empty()) m_toVal = CMaxDDword; // максимально возможное

            // x-y
            else if (!StringToDDword(*sl[1], m_toVal)) 
                ESS_THROW_MSG(RangeParserError, "Can`t convert val: " + *sl[1]);
        }
        else
        {
            m_toVal = m_fromVal;
        }        

        if (!RangeOk()) 
            ESS_THROW_MSG(RangeParserError, "Wrong range(To < From): " + s);
    }

	// ------------------------------------------------------------------------------------

	ValueRange::ValueRange( Platform::ddword from, Platform::ddword to)
	{		
		m_toVal = to;
		m_fromVal = from;
        ESS_ASSERT(RangeOk());
	}

	// ------------------------------------------------------------------------------------

    std::string ValueRange::ToString(const std::string& sep /*= "-"*/) const
    {
        if (m_fromVal == m_toVal) return ValToString(m_fromVal);
        if (m_toVal == CMaxDDword) return ValToString(m_fromVal) + sep + " ";
        return ValToString(m_fromVal) + sep + ValToString(m_toVal);
    }

    // ------------------------------------------------------------------------------------

    void ValueRange::ExpandLow( int val )
    {
        m_fromVal -= val;
        ESS_ASSERT(RangeOk());
    }

    // ------------------------------------------------------------------------------------

    void ValueRange::ExpandHi( int val )
    {
        m_toVal += val;
        ESS_ASSERT(RangeOk());
    }

    // ------------------------------------------------------------------------------------

    RangeList::RangeList( const std::string& inStr, const std::string& sep /*= ","*/, const std::string& prolong /*= "-"*/ )        
    {
        if (inStr.empty()) return;//ESS_THROW_MSG(RangeParserError, "Section is empty.");

        Utils::ManagedList<std::string> sl;        
        Utils::StringParser(inStr, sep, sl, true);
        ESS_ASSERT(!sl.IsEmpty());

        for (size_t i = 0; i < sl.Size(); ++i)
        {
            m_rangeList.push_back(ValueRange(*sl[i], prolong));
        }            
    }  

    // ------------------------------------------------------------------------------------

    RangeList* RangeList::Create( const std::string& inStr, std::string& err, const std::string& sep /*= ","*/, const std::string& prolong /*= "-"*/ )
    {
        RangeList* result = 0;
        try
        {
            result = new Utils::RangeList(inStr, sep, prolong);
        }
        catch(/*const */Utils::RangeParserError& e)
        {                       
            err = e.getTextMessage();
            return 0;
        }        
        
        return result;
    }

    // ------------------------------------------------------------------------------------

    std::string RangeList::ToString( const std::string& sep /*= ","*/, const std::string& prolong /*= "-"*/ ) const
    {
        std::string result;
        for (size_t i = 0; i < m_rangeList.size(); ++i)
        {
            result.append( m_rangeList.at(i).ToString(prolong) );
            if (i != m_rangeList.size() - 1) result.append(sep).append(" ");
        }
        return result;
    }

    // ------------------------------------------------------------------------------------

    bool RangeList::ExistValue( const Platform::ddword& val ) const
    {
        for (size_t i = 0; i < m_rangeList.size(); ++i)
        {
            if (m_rangeList.at(i).ExistValue(val)) return true;
        }
        return false;
    }

    // ------------------------------------------------------------------------------------

    const ValueRange& RangeList::operator[]( int i ) const
    {
        ESS_ASSERT(i < Size());
        return m_rangeList.at(i);
    }

    // ------------------------------------------------------------------------------------

    int RangeList::Size() const
    {
        return m_rangeList.size();
    }

    // ------------------------------------------------------------------------------------

    void RangeList::Clear()
    {
        m_rangeList.clear();
    }

    // ------------------------------------------------------------------------------------

    void ValueRange::Test() // static
    {
        TestParseRange("1-10", 1, 10);
        TestParseRange(" 1 - 10 ", 1, 10);
        TestParseRange(" 10 ", 10, 10);
        TestParseRange("10", 10, 10);

        // проверка открытого диапазона
        ddword from = CMaxDDword - 10;        
        std::string s = ValToString(from);        
        TestParseRange(s + "-", from, CMaxDDword);
        TestParseRange(" " + s + " - ", from, CMaxDDword);

        // ошибки парсинга
        TestParseRange("0fa-255", 0, 0, true); // can`t convert
        TestParseRange("0-x255", 0, 0, true);  // can`t convert        
        TestParseRange("0 = 255", 0, 0, true); // wrong separator
        TestParseRange("500-100", 0, 0, true); // ><  
    }
    
    // ------------------------------------------------------------------------------------

    void RangeList::Test() // static
    {                   
        // проверка нарезки на регионы      
        TestParseRangeList("", 0);    
        TestParseRangeList("1 - 10", 1);                
        TestParseRangeList("1 - 10,", 1);
        TestParseRangeList("1 - 10 ,", 1);
        TestParseRangeList("1 - 10 , 100", 2);
        TestParseRangeList("1 - 10, 100", 2);        
        TestParseRangeList("1 - 10 ,100", 2);        
        // wrong separator
        TestParseRangeList("1 - 10 м 100", 0, true);                                           
    }
} // namespace Utils


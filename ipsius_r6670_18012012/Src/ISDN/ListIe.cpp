#include "stdafx.h"
#include "ListIe.h"
#include "IeFactory.h"
#include "DssWarning.h"
#include "IIsdnL3Internal.h"
#include "RawData.h"

namespace ISDN
{
    using boost::shared_ptr;
    using boost::scoped_ptr;

    ListIe::ListIe(IsdnInfra& infra, IeFactory& ieFactory)
        : m_infra( infra ),
        m_ieFactory( ieFactory ),
        m_isParsed(false) 
    {            
    }

    // ----------------------------------------------------------

    void ListIe::InsertIe(ICommonIe* pIe)
    {
        ESS_ASSERT(pIe != 0);
        m_ies.Add(pIe);
    }

    // ----------------------------------------------------------

    int ListIe::GetHeaderSize(QVector<byte> l2data)
    {
        if (l2data.size() < 2) ESS_THROW(ParseError);

        int res = (l2data.at(1) & 0x0f) + 3;

        // if (res >= l2data.size()) ESS_THROW(ParseError);  // OK, if no IE in packet

        return res;
    }

    // ----------------------------------------------------------

    void ListIe::Parse(QVector<byte> l2data)
    {
        ESS_ASSERT(!m_isParsed);

        try
        {
            IeId currIe;
            int beginIePos; 

            int lockedShift = 0;
            int currPos = GetHeaderSize(l2data); 

            while( NextIe(currIe, beginIePos, l2data, currPos, lockedShift) )
            {           
                ESS_ASSERT(!currIe.Empty());
                ESS_ASSERT(beginIePos < l2data.size());

                RawData data(l2data, beginIePos);                
                InsertIe(  m_ieFactory.CreateIeObj(currIe, data)  );

                currIe.Clear();
            }

            m_isParsed = true;
        }
        catch(const ParseError &e)
        {
            shared_ptr<DssWarning> warning( PacketParseError::Create(m_infra, l2data) );
            ESS_THROW_T(DssWarningExcept, warning);
        }

    }

    // ----------------------------------------------------------

    bool ListIe::NextIe(/* out */ IeId& out, /* out */ int &beginIePos,
        const QVector<byte> &v, int &currPos, int &lockedShift)
    {        
        if ( currPos >= v.size()) return false; // is it end of packet?

        byte currVal = v.at(currPos);

        // проверка на наличие Shift 
        if ( (currVal & 0xf0) == 0x90 )
        {
            int shift = currVal & 0x07;

            // is it shift locked?
            if (!(currVal & 0x08)) lockedShift = shift;
            ++currPos;
            beginIePos = currPos;

            if (currPos >= v.size()) ESS_THROW(ParseError);

            out.Set(v.at(currPos), shift);
            return true;
        }

        // обработка однобайтовых 
        if (currVal & 0x80) // - старший бит октета ==1 те это однобайтовый ie 
        {
            int id = currVal & 0x70; // маска - 4,5,6 биты

            // error: its shift
            if (id == 1) ESS_THROW(ParseError); 

            if (id == 3 || id == 5) // Congestion level || Repeat indicator 
            {
                out.Set(id, lockedShift);
                beginIePos = currPos;
                ++currPos; 
                return true;
            }

            // (More data, Sending complete) 
            out.Set(currVal, lockedShift);
            beginIePos = currPos;
            ++currPos; 
            return true;
        }

        // обработка многобайтовых
        {
            byte id = currVal;
            beginIePos = currPos;
            ++currPos;

            if (currPos >= v.size()) ESS_THROW(ParseError); 
            int len = v.at(currPos);        

            if (beginIePos + 2 + len > v.size()) ESS_THROW(ParseError); 

            out.Set(id, lockedShift);
            currPos += len + 1; // шаг к след идентификатору        
            return true;
        }
    }

    // ----------------------------------------------------------

    void ListIe::GetExistId(std::multiset<IeId>& out) const
    {    
        // TODO -- use set or vector, not multiset
        ESS_ASSERT( out.empty() );

        for(int i = 0; i < m_ies.Size(); ++i)
        {
            out.insert( m_ies[i]->GetIeId() );
        }
    }

	// ----------------------------------------------------------

	std::string ListIe::GetAsString(bool withContent)const
	{
		if (m_ies.IsEmpty()) return "";

		std::stringstream out;

		for(int i = 0; i < m_ies.Size(); ++i)
		{
			out << m_ies[i]->GetName();
			if (withContent)
			{             
				out << " { ";
				m_ies[i]->WriteContent(out);
				out << " }; ";
			}
			else
			{
				out << ". ";
			}
			out << std::endl;
		}
		return out.str();
	}
      
} // ISDN



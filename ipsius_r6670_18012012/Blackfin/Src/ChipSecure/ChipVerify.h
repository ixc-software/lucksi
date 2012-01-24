#ifndef __CHIPVERIFY__
#define __CHIPVERIFY__

#include "Platform/Platform.h"
#include "Utils/Random.h"
#include "BfDev/BfTwi.h"
#include "Ds28cn01/ds28cn01.h"

#include "ChipUtils.h"

namespace ChipSecure
{
    using Platform::dword;
    
    // This is bad protection scheme -- program don't depend from this class
    class ChipVerify
    {
        typedef std::vector< std::pair<dword, dword> > List;

        Utils::Random m_rnd;
        BfDev::BfTwi m_twi;
        Ds28cn01::DS28CN01 m_chip;

        List m_list;  // challenge list
        std::vector<int> m_eventsList;

        dword m_lastRndRequestTime;

        dword m_statEvent;
        dword m_statRandom;

        bool RunTest(int number)
        {
            ESS_ASSERT(number < m_list.size());

            dword res = ChipUtils::ChipTransform(m_chip, m_list.at(number).first);
            return res == m_list.at(number).second;
        }

        static void Add(List &lx, dword v0, dword v1)
        {
            lx.push_back( std::pair<dword, dword>(v0, v1) );
        }

        static List MakeList();

    public:

        ESS_TYPEDEF(Error);
        
        ChipVerify(dword seed);

        bool Event(int eventNumber)
        {
            if ( std::find(m_eventsList.begin(), m_eventsList.end(), eventNumber) 
                == 
                m_eventsList.end() )
            {
                return true;
            }

            ++m_statEvent;
            int indx = m_rnd.Next( m_list.size() );
            return RunTest(indx);
        }

        std::string StatInfo() const
        {
            std::ostringstream oss;
            oss << "Req " << m_statEvent << "/" << m_statRandom;
            return oss.str();
        }

        void Process();

        static void TestAll()
        {
            ChipVerify chip(0);

            for(size_t i = 0; i < chip.m_list.size(); ++i)
            {
                TUT_ASSERT( chip.RunTest(i) );
            }
        }

    };
    
    
}  // namespace ChipSecure

#endif

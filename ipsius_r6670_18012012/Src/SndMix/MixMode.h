#ifndef __MIXMODE__
#define __MIXMODE__

#include "Utils/StringUtils.h"
#include "Utils/StringParser.h"
#include "Utils/IntToString.h"

#include "MixerError.h"

namespace SndMix
{

    enum
    {
        // максимально допустимое значение для handle (номинальное)
        CConfHandleLimit = 512,  
    };

    // --------------------------------------------------

    struct ChInput
    {
        int ChInputIndex;

        ChInput()
        {
            ChInputIndex = -1;
        }

        ChInput(int chInputIndex /* , other params */)
        {
            ESS_ASSERT(chInputIndex >= 0);
            ChInputIndex = chInputIndex;
        }

    };

    // --------------------------------------------------

    struct ChOutputDesc
    {
        int ChOutputIndex;
        std::vector<ChInput> InputChannels;

        ChOutputDesc()
        {
            ChOutputIndex = -1;
        }

        ChOutputDesc(int chOutputIndex)
        {
            ESS_ASSERT(chOutputIndex >= 0);
            ChOutputIndex = chOutputIndex;
        }

    };

    // --------------------------------------------------

    class MixMode
    {
        std::vector<ChOutputDesc> m_list;

        // cached data
        mutable bool m_changed;
        mutable std::vector<int> m_allInputChs, 
                                 m_allOutputChs;

        static void AddNonDublicate(std::vector<int> &v, int value)
        {
            std::vector<int>::iterator i = std::find(v.begin(), v.end(), value);
            if (i == v.end()) v.push_back(value);
        }

        void ValidateCacheData() const
        {
            if (!m_changed) return;

            // set m_allInputChs
            m_allInputChs.clear();

            for(int i = 0; i < m_list.size(); ++i)
            {
                const ChOutputDesc &desc = m_list.at(i);

                // desc is empty
                if (desc.ChOutputIndex < 0)
                {
                    ESS_ASSERT( desc.InputChannels.empty() );
                    continue;
                }

                for(int i = 0; i < desc.InputChannels.size(); ++i)
                {
                    AddNonDublicate( m_allInputChs, desc.InputChannels.at(i).ChInputIndex );
                }
            }

            std::sort(m_allInputChs.begin(), m_allInputChs.end());

            // set m_allOutputChs
            m_allOutputChs.clear();

            for(int i = 0; i < m_list.size(); ++i)
            {
                int ch = m_list.at(i).ChOutputIndex;
                if (ch < 0) continue;

                AddNonDublicate( m_allOutputChs, ch );
            }

            std::sort(m_allOutputChs.begin(), m_allOutputChs.end());

            // cached data OK
            m_changed = false;
        }

        bool AddChOutputMode(const ChOutputDesc &desc)
        {
            if (desc.ChOutputIndex < 0) return false;
            if (desc.ChOutputIndex >= CConfHandleLimit) return false;

            // drop cached data
            m_changed = true;

            // TODO verify desc.InputChannels: dublicates, val >= 0, no outCh inside
            // ... 

            // fill m_list with empty items
            while(desc.ChOutputIndex >= m_list.size())
            {
                m_list.push_back( ChOutputDesc() );
            }

            // set
            m_list.at(desc.ChOutputIndex) = desc;

            return true;
        }

        ESS_TYPEDEF(ParseLineError);

        void ParseLine(const std::string &line)
        {
            using namespace Utils;

            std::string s = line;
            s = TrimBlanks(s);
            if (s.empty()) return;   // next line

            int n = s.find("=");
            if (n < 0) ESS_THROW_MSG(ParseLineError, "Can't find '='");

            ChOutputDesc res;

            // parse ChOutputIndex
            {
                int val;
                if ( !StringToInt( TrimBlanks(s.substr(0, n-1)), val ) )
                {
                    ESS_THROW_MSG(ParseLineError, "Can't parse left side");
                }

                res.ChOutputIndex = val;
            }

            // parse InputChannels
            {
                Utils::ManagedList<std::string> inChannels;
                s = TrimBlanks( s.substr(n+1) );
                StringParser(s, ",", inChannels);

                for(int i = 0; i < inChannels.Size(); ++i)
                {
                    int val;
                    if ( !StringToInt( TrimBlanks(*inChannels[i]), val ) )
                    {
                        ESS_THROW_MSG(ParseLineError, "Can't parse number in right side");
                    }

                    res.InputChannels.push_back( ChInput(val) );
                }
            }

            // add
            if ( !AddChOutputMode(res) ) 
            {
                ESS_THROW_MSG(ParseLineError, "Can't add!");
            }
        }


    public:

        MixMode()
        {
            Clear();
        }

        /* Syntax example: "0 = 1,2 ; 1 = 0,2; 2 = 0, 1 " */
        MixMode(const std::string &data)
        {
            Utils::ManagedList<std::string> res;
            Utils::StringParser(data, ";", res, true);

            for(int i = 0; i < res.Size(); ++i)
            {
                try
                {
                    ParseLine( *res[i] );
                }
                catch(/* const */ ParseLineError &e)
                {
                    std::string msg = e.getTextMessage() + " @ " + *res[i];
                    ESS_THROW_MSG(Error::MixerModeParseError, msg);
                }
                
            }
        }

        void Clear()
        {
            m_changed = true;

            m_list.clear();
        }

        void ChOutputMode(const ChOutputDesc &desc)
        {
            ESS_ASSERT( AddChOutputMode(desc) );
        }

        const std::vector<int>& AllInputChs() const
        {
            ValidateCacheData();
            return m_allInputChs;
        }

        const std::vector<int>& AllOutputChs() const
        {
            ValidateCacheData();
            return m_allOutputChs;
        }

        const ChOutputDesc& OutputDesc(int chIndex) const
        {
            ESS_ASSERT(chIndex < m_list.size());
            
            const ChOutputDesc &desc = m_list.at(chIndex);
            ESS_ASSERT(desc.ChOutputIndex == chIndex);
            ESS_ASSERT(!desc.InputChannels.empty());

            return desc;
        }

    };

        
}  // namespace SndMix

#endif

#ifndef __CMDLINESPLITTER__
#define __CMDLINESPLITTER__

#include "ErrorsSubsystem.h"

namespace Utils
{
    
    class CmdLineSplitter
    {
        std::vector<std::string> m_left, m_right;
        std::vector<char*> m_leftArr, m_rightArr;

        static void MakeArr(const std::vector<std::string> &src, std::vector<char*> &dst)
        {
            dst.clear();

            for(int i = 0; i < src.size(); ++i)
            {
                char *p = (char*)src.at(i).c_str();
                dst.push_back(p);
            }

            dst.push_back(0);  // extra zero pointer at the end (not nessesary really)
        }

    public:

        CmdLineSplitter(int argc, char *argv[], std::string splitVal = "!")
        {
            ESS_ASSERT(argc >= 1);

            // deal with argv[0]
            m_left.push_back( argv[0] );
            m_right.push_back( argv[0] );

            // make m_left, m_right
            bool leftSide = true;

            for(int i = 1; i < argc; ++i)
            {
                std::string v( argv[i] );

                if (v == splitVal)
                {
                    ESS_ASSERT(leftSide);
                    leftSide = false;
                    continue;
                }

                if (leftSide) m_left.push_back(v);
                        else  m_right.push_back(v);
            }

            MakeArr(m_left, m_leftArr);
            MakeArr(m_right, m_rightArr);
        }

        int    LeftCount() const { return m_left.size(); }
        char** LeftArr()         { return &m_leftArr.at(0); }

        int    RightCount() const { return m_right.size(); }
        char** RightArr()         { return &m_rightArr.at(0); }
                
    };    
        
} // namespace Utils

#endif
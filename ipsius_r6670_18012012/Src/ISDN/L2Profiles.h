#ifndef L2PROFILES_H
#define L2PROFILES_H

#include "stdafx.h"
 
#include "Utils/ErrorsSubsystem.h"
#include "IsdnRole.h"
#include "LapdTraceOption.h"

namespace ISDN
{   
    class IsdnL2;
    using boost::shared_ptr;

    class L2Profile
    {        
        std::string m_l2Name;
        bool m_isUserSide;
        LapdTraceOption m_trace;
        bool m_fixedTei;
        bool m_orig;
        int m_tei;
        bool m_initialState1; // ST_L2_1 - if true, else ST_L2_4
        unsigned int m_maxWindowSize;
        bool m_statActive;

        L2Profile(const LapdTraceOption& trace)
            : m_trace(trace)
        {}

    public:

        void Set(IsdnL2* pL2) const;

        static shared_ptr<L2Profile> CreateAsTE(const LapdTraceOption& trace, const std::string &prefix = "",bool statActive = true)
        {
            shared_ptr<L2Profile> ret( new L2Profile(trace) );            
			
			ret->m_l2Name = prefix + "/L2_TE";
            ret->m_isUserSide = true;
            ret->m_fixedTei = true;
            ret->m_orig = true;
            ret->m_tei = -1;
            ret->m_initialState1 = true;
            ret->m_maxWindowSize = 10;
            ret->m_statActive = statActive;

            return ret;
        }

        static shared_ptr<L2Profile> CreateAsNT(const LapdTraceOption& trace, const std::string &prefix = "", bool statActive = true)
        {
            shared_ptr<L2Profile> ret( new L2Profile(trace) );
			ret->m_l2Name = prefix + "/L2_NT";
            ret->m_isUserSide = false;
            ret->m_fixedTei = false;
            ret->m_orig = false;
            ret->m_tei = 32; // default tei
            ret->m_initialState1 = false;
            ret->m_maxWindowSize = 10;
            ret->m_statActive = statActive;

            return ret;
        }

        const std::string& getLogSessionName() const
        {
            return m_l2Name;
        }

        bool getTracedOn() const
        {
            return m_trace.m_traceOn;
        }

        bool getTracedRaw() const
        {
            return m_trace.m_traceRaw;
        }

        bool getStatActive() const
        {
            return m_statActive;
        }

        void setTei(int tei)
        {
            m_tei = tei;
        }

        void setInitialState(bool initialState1)
        {
            m_initialState1 = initialState1;
        }
    };

    
} // ISDN

#endif

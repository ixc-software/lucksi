#ifndef __ECHOCANCEL__
#define __ECHOCANCEL__

#include "IEchoDebug.h"

namespace iDSP
{
    
    class EchoCancel : boost::noncopyable, public IEchoDebug
    {
        class AdoptStats;
        class EchoState;

        boost::shared_ptr<EchoState> m_echo; // scoped_ptr ?

        // OnAdoptEnter store state
        int m_adoptCounter;
        int m_adoptFactor;
        std::vector<int> *m_pCoefBeforeAdopt;  // raw pointer, change ownership
        boost::scoped_ptr<AdoptStats> m_adoptStats;

    // IEchoDebug impl
    private:

        void OnAdoptEnter(int factor);
        void OnAdoptLeave();
        
    public:

        EchoCancel(int tapsSize, bool enableAdoptDebug = false);
        ~EchoCancel();

        int Process(int tx, int rx);
      
        void GetFirCoeffs(std::vector<int> &foreground, std::vector<int> &background);

        int BgAdoptCalled() const;

        void AdoptStatToString(std::string &s);
    };
    
}  // namespace iDSP


#endif


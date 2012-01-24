#ifndef __PSBCOFIDECDRV__
#define __PSBCOFIDECDRV__

#include "Platform/Platform.h"
#include "Utils/IntToString.h"
#include "iPult/CofidecState.h"

#include "IPsbDebug.h"

namespace DevIpPult
{
    class DevIpPultSpiMng;
}

namespace PsbCofidec
{
    using namespace Platform;

    class Psb4851;
    class Psb2170;

	class PsbCofidecDrv : boost::noncopyable
	{				
        boost::scoped_ptr<DevIpPult::DevIpPultSpiMng> m_spi;

        boost::scoped_ptr<Psb4851>	m_AFE;  // analog front-end
        boost::scoped_ptr<Psb2170>	m_ACE;  // acoustic echo-canceler 

        // map value[0 .. CValMax] to range[0 .. mappedMax]
        static int Map(int value, int mappedMax)
        {
            const int CValMax = 100;
            ESS_ASSERT(mappedMax > 0);

            if (value < 0 || value > CValMax) 
            {
                ThrowError( "Bad vol value " + Utils::IntToString(value) );
            }

            int res = (value * mappedMax) / CValMax;

            ESS_ASSERT( (res >= 0) && (res <= mappedMax) );
            if (value == CValMax) ESS_ASSERT(res == mappedMax);

            return res;
        }

        static void ThrowError(const std::string &msg = "");

        void SetMode(CsEnum::CsMode mode);
        void SetLoop(CsEnum::LoopMode loopMode);

	public:

		PsbCofidecDrv(IPsbDebug *iDebug = 0);
        ~PsbCofidecDrv();

        void SetState(const iPult::CofidecState &state);
            
        void SetDebugMode(bool toHandsfree);
        
        std::string StateInfo() const;

		Psb2170 &GetACE()
		{
			return *m_ACE;
		}

        static void SetMode(CsEnum::CsMode mode, Psb4851 &afe, Psb2170 &ace);
        static void SetLoop(CsEnum::LoopMode loopMode, Psb4851 &afe, Psb2170 &ace);


	};

} //namespace PsbCofidec

#endif


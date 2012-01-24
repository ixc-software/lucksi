#ifndef __BFSYS__
#define __BFSYS__

#include <cdefBF537.h>
#include <ccblkfn.h>
#include <sys\exception.h>


namespace DevIpTdm
{
	void SecondBFLoadSpi();
    void PllInit();
	void SDRAMInit();
	void InitASYNC();	
	int SDRAMTest();
    bool GetKey();
			
};

#endif

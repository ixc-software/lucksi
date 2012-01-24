#ifndef _MINI_BOOT_LOAD_IMG_IMPL_H_
#define _MINI_BOOT_LOAD_IMG_IMPL_H_

#include "stdafx.h"
#include "MiniFlasher/MfClient.h"
#include "NObjMiniFlasher.h"
#include "MfProcessParams.h"
#include "MfProcess.h"

namespace MiniFlasher
{
    class LoadImgImpl : IMfProcessUser
	{
        int SendCompressed(MfProcess &p, const char *pData, int size);
        dword Write(MfProcess &p); // returns crc
        void Verify(MfProcess &p, dword crc, dword progressBarSize);
        void Run(MfProcess &p);
        
    // IMfProcessUser impl;
    private:
        void RunImpl(MfProcess &p); // can throw
        
    public:
        // Required parameters:
        // - uart profile
        // - file name
        // - flash offset
        // - flash erase size == loaded file size
		LoadImgImpl(MfProcessParams params);
	};
    
} // namespace

#endif

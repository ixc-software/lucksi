#include "stdafx.h"
#include "VdkCheckInit.h"
#include "Utils/ErrorsSubsystem.h"

namespace
{
	const int CMagic = 0xF0104515;
	
	int GMagic;
	
}  // namespace

namespace iVDK
{
	
	void VdkInitDone()
	{
		GMagic = CMagic;
	}
	
	void VdkCheckInit()
	{
		ESS_ASSERT(GMagic == CMagic);
	}
	
		
}  // namespace iVDK

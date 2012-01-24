#include "stdafx.h"
#include "ExeName.h"
#include "ErrorsSubsystem.h"

using std::string;

namespace
{
	bool GInitDone = false;
    std::string GName = "Blackfin.exe";
    std::string GDir  = "";
}

namespace Utils
{
	
	void ExeName::Init(const char *pExeName)
	{
	    ESS_ASSERT(!GInitDone); 
	
	    GInitDone = true;
	}
		
	const string& ExeName::GetExeName()
	{
	    return GName;
	}
	
	// ----------------------------------------------------
	
	const string& ExeName::GetExeDir()
	{
	    return GDir;
	}
	
		
}  // namespace Utils




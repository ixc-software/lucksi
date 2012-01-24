#include "stdafx.h"

#include <VDK.h>

#include "Utils/ErrorsSubsystem.h"
#include "Platform/PlatformTypes.h"

#include "BfDev/SysProperties.h"

#include "Src/UtilsTests/MemoryPoolTests.h"


//-----------------------------------------------------------------------------


using namespace std;
using namespace Platform;
using namespace UtilsTests;


//-----------------------------------------------------------------------------


void MainBody()
{
    FixedMemBlockPoolTest::Test();
    cout << "." << endl;

    FixedMemBlockPoolTest::UsingSimulation();
    cout << "." << endl;
}

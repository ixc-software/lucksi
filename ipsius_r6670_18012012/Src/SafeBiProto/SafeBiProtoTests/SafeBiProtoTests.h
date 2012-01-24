
#ifndef __SAFEBIPROTOTESTS__
#define __SAFEBIPROTOTESTS__

// SafeBiProtoTests.h
#include "ITestTransportsCreator.h"
#include "SafeBiProto/SbpTypes.h"

namespace SBPTests
{
    void SafeBiProtoTestSimple(ITestTransportsCreator &creator, bool testWithMsgs, 
                               bool silentMode = true);
    void SafeBiProtoTest(ITestTransportsCreator &creator, bool testWithMsgs, 
                         bool silentMode = true);
    
    void SbpUtilsTests(bool silentMode = true);
    void SbpRecvParsingTest(bool silentMode = true);

    // Using to monitor memory leak.
    // Each side (clent and server) send 'packCount' packets to each other.
    // iterations - number of cycles (client -> server, server -> client)
    // timeout - sending process timeout
    void SbpPackExchange(ITestTransportsCreator &creator, bool testWithMsgs, 
                         word packCount, byte iterations, dword timeoutMs);

    void SbpRecvJunkDataTest(dword sentPartsCount, dword maxSentPart,
                             dword timeout, bool showErrLog, bool silentMode);

    void SbpExceptionsTest(bool silentMode = true);
     
} // namespaceSBPTests 

#endif
